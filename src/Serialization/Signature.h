#pragma once

#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <vector>
//

#include <Serialization/Crc32.h>
#include <Serialization/Traits.h>
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
			constexpr auto members = filter(refl::type_descriptor<Type>::members, [](auto member) { return Traits::is_field(member); });
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
			constexpr auto members = filter(refl::type_descriptor<Type>::members, [](auto member) { return Traits::is_field(member); });
			return refl::util::accumulate(
				members, [](const auto checksum, const auto member) { return checksum ^ MakeMemberChecksum(member); }, Checksum());
		}

	} // namespace Utils::Signature
} // namespace Grafkit
