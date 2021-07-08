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

		namespace Impl
		{

			// ---
			template <typename MemberType> static constexpr auto MakeFieldLine(const MemberType & member)
			{
				return type_name<typename MemberType::value_type>::value + refl::make_const_string(' ') + member.name + refl::make_const_string("; ");
			}

			template <typename ParentType, typename FunctionType> static constexpr auto MakePropertyLine(const FunctionType & member)
			{
				return type_name<typename FunctionType::return_type<ParentType>>::value + refl::make_const_string(' ') +
					   refl::descriptor::get_display_name_const(member) + refl::make_const_string("; ");
			}

			template <typename Type> static constexpr auto CalcStringFields()
			{
				constexpr auto members = filter(refl::type_descriptor<Type>::members, [](auto member) { return Traits::is_serializable_field(member); });
				return refl::util::accumulate(
					members, [](const auto accumulated, const auto member) { return accumulated + MakeFieldLine(member); }, refl::make_const_string());
			};

			template <typename Type> static constexpr auto CalcStringProperties()
			{
				constexpr auto members = filter(refl::type_descriptor<Type>::members, [](auto member) { return Traits::is_serializable_getter(member); });
				return refl::util::accumulate(
					members, [](const auto accumulated, const auto member) { return accumulated + MakePropertyLine<Type>(member); }, refl::make_const_string());
			};

		} // namespace Impl

		// TODO: Remove trailing space
		template <typename Type> static constexpr auto CalcString() { return Impl::CalcStringFields<Type>() + Impl::CalcStringProperties<Type>(); }

		namespace Impl
		{

			template <typename MemberType> static constexpr Checksum MakeFieldChecksum(const MemberType & member)
			{
				return Checksum(type_name<typename MemberType::value_type>::value.data) ^ Checksum(' ') ^ Checksum(member.name.data) ^ Checksum("; ");
			}

			template <typename ParentType, typename MemberType> static constexpr Checksum MakePropertyChecksum(const MemberType & member)
			{
				return Checksum(type_name<typename MemberType::return_type<ParentType>>::value.data) ^ Checksum(' ') ^
					   Checksum(refl::descriptor::get_display_name_const(member).data) ^ Checksum("; ");
			}

			template <typename Type> static constexpr Checksum CalcFieldChecksum()
			{
				constexpr auto members = filter(refl::type_descriptor<Type>::members, [](auto member) { return Traits::is_serializable_field(member); });
				return refl::util::accumulate(
					members, [](const auto checksum, const auto member) { return checksum ^ MakeFieldChecksum(member); }, Checksum());
			}

			template <typename Type> static constexpr Checksum CalcPropertyChecksum()
			{
				constexpr auto members = filter(refl::type_descriptor<Type>::members, [](auto member) { return Traits::is_serializable_getter(member); });
				return refl::util::accumulate(
					members, [](const auto checksum, const auto member) { return checksum ^ MakePropertyChecksum<Type>(member); }, Checksum());
			}

		} // namespace Impl

		// TODO: Remove trailing space

		template <typename Type> static constexpr Checksum CalcChecksum() { return Impl::CalcFieldChecksum<Type>() ^ Impl::CalcPropertyChecksum<Type>(); }

	} // namespace Utils::Signature
} // namespace Grafkit
