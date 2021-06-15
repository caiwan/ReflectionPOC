#pragma once

#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <vector>
//

#include <Serialization/Crc32.h>
#include <refl.h>

/**
 * Reflection signature utilities
 * + Basic type -> string + enum (CRC?)
 * + Concat member names
 * + Recurse traversal?
 * + Add API to write own schema file (json, xml, txt, native bin?)
 */

namespace Grafkit
{
	// TODO These could be extracted
	namespace Attributes
	{

		/**
		 * Marker for fields to be serialized
		 */
		struct Serializable : refl::attr::usage::field
		{
		};

		/**
		 * Members to be called before serialization begins
		 */
		struct OnBeforeSerialize : refl::attr::usage::function
		{
			// TODO: Read, write, readwrite
		};

		/**
		 * Members to be called after serialization is done
		 */
		struct OnAfterSerialize : refl::attr::usage::function
		{
			// TODO: Read, write, readwrite
		};

		/**
		 * Marker for types to be registered to be dynamically constructed
		 */
		struct RegisteredType : refl::attr::usage::type
		{
		};

	} // namespace Attributes

	// TODO Test all of these
	namespace Traits
	{
		/**
		 * Is shared ptr
		 * @tparam T
		 */
		template <typename T> struct is_shared_ptr : std::false_type
		{
		};

		template <typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type
		{
		};

		template <typename T> static constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;

		/**
		 * Is unique ptr
		 * @tparam T
		 */
		template <typename T> struct is_unique_ptr : std::false_type
		{
		};

		template <typename T> struct is_unique_ptr<std::unique_ptr<T>> : std::true_type
		{
		};

		template <typename T> static constexpr bool is_unique_ptr_v = is_unique_ptr<T>::value;

		/**
		 * Is iterable
		 * @tparam T
		 */
		template <typename T, typename = void> struct is_iterable : std::false_type
		{
		};

		template <typename T> struct is_iterable<T, std::void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>> : std::true_type
		{
		};

		template <typename T> constexpr bool is_iterable_v = is_iterable<T>::value;

#if _MSC_VER
		/*
		 * Workaround for MSVC lack of proper SFINAE support.
		 * https://stackoverflow.com/questions/87372/check-if-a-class-has-a-member-function-of-a-given-signature/10707822#10707822
		 */
#	define GK_MSVC_SFINAE_HAS_FN_WORKAROUND(NAME)                                                                                                             \
		template <typename, typename T> struct test_has_##NAME                                                                                                 \
		{                                                                                                                                                      \
			static_assert(std::integral_constant<T, false>::value, "Second template parameter needs to be a function type.");                                  \
		};                                                                                                                                                     \
                                                                                                                                                               \
		template <typename C, typename Ret, typename... Args> struct test_has_##NAME<C, Ret(Args...)>                                                          \
		{                                                                                                                                                      \
		private:                                                                                                                                               \
			template <typename T>                                                                                                                              \
			static constexpr auto check(T *) -> typename std::is_same<decltype(std::declval<T>().##NAME(std::declval<Args>()...)), Ret>::type;                 \
			template <typename> static constexpr std::false_type check(...);                                                                                   \
			typedef decltype(check<C>(0)) type;                                                                                                                \
                                                                                                                                                               \
		public:                                                                                                                                                \
			static constexpr bool value = type::value;                                                                                                         \
		}

#endif

#ifndef GK_MSVC_SFINAE_HAS_FN_WORKAROUND

		/**
		 * Has size()
		 * @tparam T
		 */
		template <typename T, typename = void> struct has_size : std::false_type
		{
		};

		template <typename T> struct has_size<T, std::void_t<decltype(std::declval<T>().size())>> : std::true_type
		{
		};

#else
		GK_MSVC_SFINAE_HAS_FN_WORKAROUND(size);
		template <typename T> using has_size = test_has_size<T, size_t(void)>;

#endif

		template <typename T> constexpr bool has_size_v = has_size<T>::value;

#ifndef GK_MSVC_SFINAE_HAS_FN_WORKAROUND
		/**
		 * Has push_back
		 * @tparam T
		 */

		template <typename T, typename U, typename = void> struct has_push_back : std::false_type
		{
		};

		// Note: With static_cast you can check for exact parameters and return type as well
		//
		// Note: But also, std::declval() does not care about implicit conversions and ignores return type, like so:
		// std::void_t<decltype(std::declval<T>().insert(std::declval<U>()))>> : std::true_type
		template <typename T, typename U> struct has_push_back<T, U, std::void_t<decltype(static_cast<void (T::*)(const U &)>(&T::push_back))>> : std::true_type
		{
		};

#else
		GK_MSVC_SFINAE_HAS_FN_WORKAROUND(push_back);
		template <typename T, typename U> using has_push_back = test_has_push_back<T, void(U)>;

#endif

		template <typename T, typename U> constexpr bool has_push_back_v = has_push_back<T, U>::value;

#ifndef GK_MSVC_SFINAE_HAS_FN_WORKAROUND

		/**
		 * Has insert
		 * @tparam T
		 */
		template <typename T, typename U, typename = void> struct has_insert : std::false_type
		{
		};

		template <typename T, typename U>
		struct has_insert<T, U,
			// Note: Usually std insert returns with a pair of result and the iterator. This has to be checked as well
			std::void_t<decltype(static_cast<std::pair<typename T::iterator, bool> (T::*)(const U &)>(&T::insert))>> : std::true_type
		{
		};

#else
		GK_MSVC_SFINAE_HAS_FN_WORKAROUND(insert);
		template <typename T, typename U>
		using has_insert =
			std::disjunction<test_has_insert<T, std::pair<typename T::iterator, bool>(U)>, test_has_insert<T, typename T::iterator(U)>, std::false_type>;

#endif
		template <typename T, typename U> constexpr bool has_insert_v = has_insert<T, U>::value;

		/**
		 * Has insert
		 * @tparam T
		 */
		template <typename T, typename = void> struct is_pair : std::false_type
		{
		};

		template <typename T> struct is_pair<T, std::void_t<decltype(std::declval<T>().first), decltype(std::declval<T>().second)>> : std::true_type
		{
		};

		template <typename T>[[maybe_unused]] static constexpr bool is_pair_v{is_pair<T>::value};

		/**
		 * Is string
		 * @tparam T
		 */
		template <typename T> struct is_string_type : std::false_type
		{
		};

		template <typename T> struct is_string_type<std::basic_string<T>> : std::true_type
		{
		};

		template <typename T> static constexpr bool is_string_type_v = is_string_type<T>::value;

		/**
		 * Check if either raw ptr, shared or unique ptr.
		 * @tparam T
		 */

		template <typename T> struct is_pointer_like : std::integral_constant<bool, std::is_pointer_v<T> || is_unique_ptr_v<T> || is_shared_ptr_v<T>>
		{
		};

		template <typename T> static constexpr bool is_pointer_like_v = is_pointer_like<T>::value;

		/**
		 *
		 */
		// TODO: This has to be renamed to something else
		template <typename T> static constexpr bool is_reflectable_field(const T)
		{
			return refl::trait::is_field_v<T> && refl::descriptor::is_readable(T({})) && refl::descriptor::is_writable(T({})) ||
				   refl::trait::is_reflectable_v<T> && refl::descriptor::has_attribute<Attributes::Serializable>(T({}));
		}

		template <typename T> static constexpr bool is_serializable(const T) { return refl::descriptor::has_attribute<Attributes::Serializable>(T({})); }

	} // namespace Traits

	namespace Utils::Signature
	{

		enum class ETypeIdentifier
		{
			Invalid = 0,
			Bool,
			UnsignedChar,
			Char,
			Short,
			UnsignedShort,
			Int,
			UnsignedInt,
			Long,
			UnsignedLong,
			LongLong,
			UnsignedLongLong,
			Float,
			Double,
			String,
			Container,
			Map,
			Struct,
		};

		template <typename T, typename = void> struct type_name
		{
			static constexpr auto myName()
			{
				if constexpr (refl::trait::is_reflectable_v<T>)
				{
					return refl::reflect<T>().name;
				}
				else
				{
					return refl::make_const_string("<unknown>");
				}
			}

			static constexpr auto value = type_name::myName();
			static constexpr ETypeIdentifier typeIdentifier = ETypeIdentifier::Invalid;
		};

#define GK_CREATE_TYPE_NAME_RESOLVE_BASE_TYPES(TYPE, ID)                                                                                                       \
	template <> struct type_name<TYPE>                                                                                                                         \
	{                                                                                                                                                          \
		static constexpr refl::const_string value = refl::make_const_string(#TYPE);                                                                            \
		static constexpr Grafkit::Utils::Signature::ETypeIdentifier typeIdentifier = ID;                                                                       \
	}

		GK_CREATE_TYPE_NAME_RESOLVE_BASE_TYPES(bool, Grafkit::Utils::Signature::ETypeIdentifier::Bool);
		GK_CREATE_TYPE_NAME_RESOLVE_BASE_TYPES(char, Grafkit::Utils::Signature::ETypeIdentifier::Char);
		GK_CREATE_TYPE_NAME_RESOLVE_BASE_TYPES(unsigned char, Grafkit::Utils::Signature::ETypeIdentifier::UnsignedChar);
		GK_CREATE_TYPE_NAME_RESOLVE_BASE_TYPES(short, Grafkit::Utils::Signature::ETypeIdentifier::Short);
		GK_CREATE_TYPE_NAME_RESOLVE_BASE_TYPES(unsigned short, Grafkit::Utils::Signature::ETypeIdentifier::UnsignedShort);
		GK_CREATE_TYPE_NAME_RESOLVE_BASE_TYPES(int, Grafkit::Utils::Signature::ETypeIdentifier::Int);
		GK_CREATE_TYPE_NAME_RESOLVE_BASE_TYPES(unsigned int, Grafkit::Utils::Signature::ETypeIdentifier::UnsignedInt);
		GK_CREATE_TYPE_NAME_RESOLVE_BASE_TYPES(long, Grafkit::Utils::Signature::ETypeIdentifier::Long);
		GK_CREATE_TYPE_NAME_RESOLVE_BASE_TYPES(long long, Grafkit::Utils::Signature::ETypeIdentifier::LongLong);
		GK_CREATE_TYPE_NAME_RESOLVE_BASE_TYPES(unsigned long, Grafkit::Utils::Signature::ETypeIdentifier::UnsignedLong);
		GK_CREATE_TYPE_NAME_RESOLVE_BASE_TYPES(unsigned long long, Grafkit::Utils::Signature::ETypeIdentifier::UnsignedLongLong);
		GK_CREATE_TYPE_NAME_RESOLVE_BASE_TYPES(float, Grafkit::Utils::Signature::ETypeIdentifier::Float);
		GK_CREATE_TYPE_NAME_RESOLVE_BASE_TYPES(double, Grafkit::Utils::Signature::ETypeIdentifier::Double);
		GK_CREATE_TYPE_NAME_RESOLVE_BASE_TYPES(std::string, Grafkit::Utils::Signature::ETypeIdentifier::String);

#define GK_CREATE_TYPE_NAME_RESOLVE_STL(TYPE, ID)                                                                                                              \
	template <typename T> struct type_name<TYPE<T>>                                                                                                            \
	{                                                                                                                                                          \
		static constexpr refl::const_string value =                                                                                                            \
			refl::make_const_string(#TYPE) + refl::make_const_string('<') + Grafkit::Utils::Signature::type_name<T>::value + refl::make_const_string('>');     \
		static constexpr Grafkit::Utils::Signature::ETypeIdentifier typeIdentifier = ID;                                                                       \
	}

#define GK_CREATE_TYPE_NAME_RESOLVE_STL_2(TYPE, ID)                                                                                                            \
	template <typename T, typename U> struct type_name<TYPE<T, U>>                                                                                             \
	{                                                                                                                                                          \
		static constexpr refl::const_string value = refl::make_const_string(#TYPE) + refl::make_const_string('<') +                                            \
													Grafkit::Utils::Signature::type_name<T>::value + refl::make_const_string(", ") +                           \
													Grafkit::Utils::Signature::type_name<U>::value + refl::make_const_string('>');                             \
		static constexpr Grafkit::Utils::Signature::ETypeIdentifier typeIdentifier = ID;                                                                       \
	}

		GK_CREATE_TYPE_NAME_RESOLVE_STL(std::list, Grafkit::Utils::Signature::ETypeIdentifier::Container);
		GK_CREATE_TYPE_NAME_RESOLVE_STL(std::vector, Grafkit::Utils::Signature::ETypeIdentifier::Container);
		GK_CREATE_TYPE_NAME_RESOLVE_STL(std::deque, Grafkit::Utils::Signature::ETypeIdentifier::Container);
		GK_CREATE_TYPE_NAME_RESOLVE_STL(std::set, Grafkit::Utils::Signature::ETypeIdentifier::Container);
		GK_CREATE_TYPE_NAME_RESOLVE_STL(std::multiset, Grafkit::Utils::Signature::ETypeIdentifier::Container);
		GK_CREATE_TYPE_NAME_RESOLVE_STL_2(std::map, Grafkit::Utils::Signature::ETypeIdentifier::Map);
		// GK_CREATE_TYPE_NAME_RESOLVE_STL_2(std::unordered_map, Grafkit::Utils::Signature::ETypeIdentifier::Map); // TODO
		GK_CREATE_TYPE_NAME_RESOLVE_STL_2(std::multimap, Grafkit::Utils::Signature::ETypeIdentifier::Map);
		// GK_CREATE_TYPE_NAME_RESOLVE_STL_2(std::unordered_multimap, Grafkit::Utils::Signature::ETypeIdentifier::Map); // TODO

#undef GK_CREATE_TYPE_NAME_RESOLVE_BASE_TYPES
#undef GK_CREATE_TYPE_NAME_RESOLVE_STL
#undef GK_CREATE_TYPE_NAME_RESOLVE_STL_2

		// ---
		template <typename MemberType> static constexpr auto MakeMemberLine(const MemberType & member)
		{
			// TODO: value_type + member_type - use whichever is available
			return type_name<typename MemberType::value_type>::value + refl::make_const_string(' ') + member.name + refl::make_const_string("; ");
		}

		template <typename Type> static constexpr auto CalcString()
		{
			constexpr auto members = filter(refl::type_descriptor<Type>::members, [](auto member) { return Traits::is_serializable(member); });
			return refl::util::accumulate(
				members, [](const auto accumulated, const auto member) { return accumulated + MakeMemberLine(member); }, refl::make_const_string());
			// TODO: Remove trailing space
		};

		// ---
		template <typename MemberType> static constexpr Checksum MakeMemberChecksum(const MemberType & member)
		{
			// TODO: has_value_type + member_type - use whichever is available
			return Checksum(type_name<typename MemberType::value_type>::value.data) ^ Checksum(' ') ^ Checksum(member.name.data) ^ Checksum("; ");
		}

		template <typename Type> static constexpr Checksum CalcChecksum()
		{
			constexpr auto members = filter(refl::type_descriptor<Type>::members, [](auto member) { return Traits::is_serializable(member); });
			return refl::util::accumulate(
				members, [](const auto checksum, const auto member) { return checksum ^ MakeMemberChecksum(member); }, Checksum());
		}

	} // namespace Utils::Signature
} // namespace Grafkit
