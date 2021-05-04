#include <algorithm>
#include <type_traits>
//
#include <nlohmann/json.hpp>
#include <refl.h>
//
#include <Serialization/Crc32.h>
#include <Serialization/Stream.h>

namespace Grafkit
{
	using Json = nlohmann::json;

	namespace Serializer
	{
		class JsonAdapter
		{
		public:
			explicit JsonAdapter(Json & json) : json(json) {}
			explicit JsonAdapter(Json && json) : json(json) {}

			static Json ParseJson(IStream & stream);

			static void DumpJson(IStream & stream, const Json & json);

		protected:
			template <typename Type> void Write(const Type & value) const { Write(value, json); }

			template <class Type, size_t N> void Write(const Type (&value)[N]) const { Write(value, json); }

			template <class Type, size_t N> void Write(const std::array<Type, N> & value) const { Write(value, json); }

			template <typename Type> void Read(Type & value) { Read(value, json); }

			template <class Type, size_t N> void Read(Type (&value)[N]) { Read(value, json); }

			template <class Type, size_t N> void Read(std::array<Type, N> & value) { Read(value, json); }

			// ----------------------------------------------------------------------------

			template <typename Type> void Write(const Type & value, Json & jsonNode) const
			{
				// --
				if constexpr (std::is_arithmetic_v<Type> || Traits::is_string_type_v<Type>) { jsonNode = value; }
				else if constexpr (std::is_enum_v<Type>)
				{
					jsonNode = static_cast<int>(value);
				}
				// ---
				else if constexpr (Traits::is_pointer_like_v<Type>)
				{
					// TODO Check if not a pointer to an arithmetic type
					// TODO Invoke persistence API
				}

				// --- STL-like container support
				else if constexpr (Traits::is_iterable_v<Type>)
				{
					jsonNode = Json::array();
					static_assert(Traits::has_size_v<Type>);
					for (const auto & elem : value) { Write(elem, jsonNode.emplace_back()); }
				}
				else if constexpr (Traits::is_pair_v<Type>)
				{
					jsonNode = Json::array();
					Write(value.first, jsonNode.emplace_back());
					Write(value.second, jsonNode.emplace_back());
				}

				// --- The rest of the stuff which has reflection data attached
				else
				{
					constexpr auto checksum = Utils::Signature::CalcChecksum<Type>();
					jsonNode = {};
					jsonNode["_checksum"] = (Utils::Checksum::ChecksumType)checksum;

					refl::util::for_each(refl::reflect(value).members, [&](auto member) {
						if constexpr (Traits::is_reflectable(member))
						{
							const auto & memberValue = member(value);
							jsonNode[member.name.c_str()] = {};
							Write(memberValue, jsonNode[member.name.c_str()]);
						}
					});
				}
			}

			template <class Type, size_t N> void Write(const Type (&value)[N], Json & jsonNode) const
			{
				jsonNode = Json::array();
				for (const auto & elem : value)
				{
					jsonNode.emplace_back();
					Write(elem, jsonNode.back());
				}
			}

			template <class Type, size_t N> void Write(const std::array<Type, N> & value, Json & jsonNode) const
			{
				jsonNode = Json::array();
				for (const auto & elem : value)
				{
					jsonNode.emplace_back();
					Write(value, jsonNode.back());
				}
			}

			// ----------------------------------------------------------------------------

			// ---
			template <typename Type> void Read(Type & value, const Json & jsonNode)
			{
				// ---
				if constexpr (std::is_arithmetic_v<Type> || Traits::is_string_type_v<Type>) { value = jsonNode.get<Type>(); }
				else if constexpr (std::is_enum_v<Type>)
				{
					value = static_cast<Type>(jsonNode.get<int>());
				}

				// ---
				else if constexpr (Traits::is_pointer_like_v<Type>)
				{
					// TODO Check if not a pointer to an arithmetic type
					// TODO Invoke persistence API
				}

				// STL-like container support
				else if constexpr (Traits::is_iterable_v<Type>)
				{
					static_assert(Traits::has_size_v<Type>);

					const auto count = jsonNode.size();
					using ValueType = typename Type::value_type;

					// TODO: assert if has value type
					using ValueType = typename Type::value_type;
					for (size_t i = 0; i < count; ++i)
					{
						ValueType readValue = {};
						Read(readValue, jsonNode.at(i));

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

					Read(first, jsonNode.at(0));
					Read(second, jsonNode.at(1));

					// Assignment operator is deleted when either `first` or `second` is not copy-assignable.
					// So we need to take care of this some way.
					(*const_cast<FirstType *>(&(value.first))) = std::move(first);
					(*const_cast<SecondType *>(&(value.second))) = std::move(second);
				}

				// -- The rest of the stuff which has reflection data attached
				else
				{
					constexpr auto checksum = Utils::Signature::CalcChecksum<Type>();
					Utils::Checksum readChecksum(jsonNode["_checksum"].get<Utils::Checksum::ChecksumType>());

					if (checksum != readChecksum) { throw std::runtime_error("Checksum does not match"); }

					refl::util::for_each(refl::reflect(value).members, [&](auto member) {
						if constexpr (Traits::is_reflectable(member))
						{
							auto & memberValue = member(value);
							Read(memberValue, jsonNode[member.name.c_str()]);
						}
					});
				}
			}

			template <class T, size_t N> void Read(T (&value)[N], const Json & jsonNode)
			{
				size_t i = 0;
				for (auto & elem : value)
				{
					Read(elem, jsonNode.at(i));
					++i;
				}
			}

			template <class T, size_t N> void Read(std::array<T, N> & value, const Json & jsonNode)
			{
				size_t i = 0;
				for (auto & elem : value)
				{
					Read(elem, jsonNode.at(i));
					++i;
				}
			}

		private:
			Json & json;
		};

	} // namespace Serializer
} // namespace Grafkit
