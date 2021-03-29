#include <algorithm>
#include <type_traits>
//
#include <refl.h>
//
#include <Serialization/Crc32.h>
//
#include <Serialization/EndianSwapper.h>
#include <Serialization/Signature.h>
#include <Serialization/Stream.h>

namespace Grafkit::Serializer
{
	class BinaryAdapter
	{
	public:
		using SizeType = uint64_t;

		static constexpr size_t StringBufferSize = 4096;

		// TODO: Get rid of std::unique_ptr

		explicit BinaryAdapter(IStream & stream) : stream(stream) {}
		explicit BinaryAdapter(IStream && stream) : stream(stream) {}

		// https://github.com/veselink1/refl-cpp/issues/29

	protected:
		// Write
		template <typename Type> void Write(const Type & value) const
		{
			assert(stream);

			// --
			if constexpr (std::is_arithmetic_v<Type>)
			{
				const Type endianCorrectedValue = Swap(value);
				stream.Write((const char *)&endianCorrectedValue, sizeof(Type));
				assert(stream.IsSuccess()); // Todo: throw error
			}
			else if constexpr (std::is_enum_v<Type>)
			{
				const int intValue = static_cast<int>(value);
				Write(intValue);
			}

			// ---
			else if constexpr (Traits::is_pointer_like_v<Type>)
			{
				// TODO Check if not a pointer to an arithmetic type
				// TODO Invoke persistence API
			}

			// --- Wired-in std::string (and any other string-based type) support
			else if constexpr (Traits::is_string_type_v<Type>)
			{
				// TODO:: Assert if has value_type
				using CharType = typename Type::value_type;
				constexpr auto charSize = sizeof(CharType);
				const auto length = static_cast<SizeType>(value.length() + 1);
				Write(length);
				stream.Write(value.c_str(), charSize * length);
			}

			// --- STL-like container support
			else if constexpr (Traits::is_iterable_v<Type>)
			{
				static_assert(Traits::has_size_v<Type> != 0);
				const auto count = static_cast<SizeType>(value.size());
				Write(count);
				for (const auto & elem : value) { Write(elem); }
			}
			else if constexpr (Traits::is_pair_v<Type>)
			{
				Write(value.first);
				Write(value.second);
			}

			// --- The rest of the stuff which has reflection data attached
			else
			{
				constexpr auto checksum = Utils::Signature::CalcChecksum<Type>();
				Write(checksum);

				refl::util::for_each(refl::reflect(value).members, [&](auto member) {
					if constexpr (Traits::is_reflectable(member))
					{
						const auto & memberValue = member(value);
						Write(memberValue);
					}
				});
			}
		}

		// Array support
		template <class T, size_t N> void Write(const T (&value)[N]) const
		{
			const auto length = static_cast<SizeType>(N);
			Write(length);
			for (const auto & item : value) Write(item);
		}

		template <class T, size_t N> void Write(const std::array<T, N> & value) const
		{
			const auto length = static_cast<SizeType>(N);
			Write(length);
			for (const auto & item : value) Write(item);
		}

		// --------------------------------------------------------

		// Read
		template <typename Type> void Read(Type & value)
		{
			assert(stream);
			// ---
			if constexpr (std::is_arithmetic_v<Type>)
			{
				const auto lastPos = static_cast<std::istream &>(stream).tellg();
				Type readValue = {};
				stream.Read((char *)(&readValue), sizeof(Type)); // TODO This part is dangerous
				if (!stream.IsSuccess()) throw std::runtime_error("malformed data - lastPos: " + std::to_string(lastPos));
				value = Swap(readValue);
			}
			else if constexpr (std::is_enum_v<Type>)
			{
				int intValue = 0;
				Read(intValue);
				value = static_cast<Type>(intValue);
			}

			// ---
			else if constexpr (Traits::is_pointer_like_v<Type>)
			{
				// TODO Check if not a pointer to an arithmetic type
				// TODO Invoke persistence API
			}

			// -- Wired-in std::string support
			else if constexpr (Traits::is_string_type_v<Type>)
			{
				// TODO: assert if has value_type
				using CharType = typename Type::value_type;

				SizeType length = 0;
				Read(length);

				value.clear();
				CharType buffer[StringBufferSize];
				SizeType toRead = length * sizeof(CharType);
				while (toRead != 0)
				{
					const auto readLength = std::min(toRead, static_cast<SizeType>(sizeof(buffer)));
					stream.Read(buffer, readLength);

					value += std::string(buffer, readLength);
					toRead -= readLength;
				}
				// remove  trailing zero we stored, but don't need
				value.pop_back();
			}

			// STL-like container support
			else if constexpr (Traits::is_iterable_v<Type>)
			{
				static_assert(Traits::has_size_v<Type>);

				SizeType count = 0;
				Read(count);

				using ValueType = typename Type::value_type;

				// TODO: assert if has value type
				using ValueType = typename Type::value_type;
				for (SizeType i = 0; i < count; ++i)
				{
					ValueType readValue = {};
					Read(readValue);

					if constexpr (Traits::has_push_back_v<Type, ValueType>) { value.push_back(std::move(readValue)); }
					else if constexpr (Traits::has_insert_v<Type, ValueType>)
					{
						value.insert(std::move(readValue));
					}
				}
			}
			else if constexpr (Traits::is_pair_v<Type>)
			{
				// Todo this whole could be solved if it would just `return` every value
				// Sometimes pairs comes as const, especially `first` which used as keys in maps
				// So we need to reconstruct it
				using FirstType = std::remove_const_t<decltype(value.first)>;
				using SecondType = std::remove_const_t<decltype(value.second)>;

				FirstType first;
				SecondType second;

				Read(first);
				Read(second);

				// Assignment operator is deleted when either `first` or `second` is not copy-assignable.
				// So we need to take care of this some way.
				(*const_cast<FirstType *>(&(value.first))) = std::move(first);
				(*const_cast<SecondType *>(&(value.second))) = std::move(second);
			}

			// -- The rest of the stuff which has reflection data attached
			else
			{
				constexpr auto checksum = Utils::Signature::CalcChecksum<Type>();



				Utils::Checksum readChecksum = 0;
				Read(readChecksum);

				if (checksum != readChecksum) { throw std::runtime_error("Checksum does not match"); }

				refl::util::for_each(refl::reflect(value).members, [&](auto member) {
					if constexpr (Traits::is_reflectable(member))
					{
						// using MemberType = std::remove_reference_t<decltype(member(value))>;
						auto & memberValue = member(value);
						Read(memberValue);
					}
				});
			}
		}

		template <class T, size_t N> void Read(T (&value)[N])
		{
			SizeType length = 0;
			Read(length);
			assert(length == N); // Todo: throw error
			for (auto & item : value) Read(item);
		}

		template <class T, size_t N> void Read(std::array<T, N> & value)
		{
			SizeType length = 0;
			Read(length);
			assert(length == N); // Todo: throw error
			for (auto & item : value) Read(item);
		}

	private:
		// TODO: Invoke Persistence here

		template <class T>[[nodiscard]] T Swap(const T & v) const { return EndianSwapper::SwapByte<T, sizeof(T)>::Swap(v); }

		IStream & stream;

		// ---
		// SizeType has to be compatible, but not equal to size_t
		// Otherwise we would have to store the size_t length in the file and take extra measures.
		static_assert(sizeof(SizeType) >= sizeof(size_t), "SizeType has to be compatible with size_t");
	};
} // namespace Grafkit::Serializer
