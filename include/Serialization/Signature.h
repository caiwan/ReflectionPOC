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

		template <typename T, typename = void> struct FindTypeName
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

			static constexpr auto value = FindTypeName::myName();
			static constexpr ETypeIdentifier typeIdentifier = ETypeIdentifier::Invalid;
		};

#define GK_CREATE_TYPE_NAME_RESOLVE_BASE_TYPES(TYPE, ID)                                                                                                       \
	template <> struct FindTypeName<TYPE>                                                                                                                      \
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
	template <typename T> struct FindTypeName<TYPE<T>>                                                                                                         \
	{                                                                                                                                                          \
		static constexpr refl::const_string value =                                                                                                            \
			refl::make_const_string(#TYPE) + refl::make_const_string('<') + Grafkit::Utils::Signature::FindTypeName<T>::value + refl::make_const_string('>');  \
		static constexpr Grafkit::Utils::Signature::ETypeIdentifier typeIdentifier = ID;                                                                       \
	}

#define GK_CREATE_TYPE_NAME_RESOLVE_STL_2(TYPE, ID)                                                                                                            \
	template <typename T, typename U> struct FindTypeName<TYPE<T, U>>                                                                                          \
	{                                                                                                                                                          \
		static constexpr refl::const_string value = refl::make_const_string(#TYPE) + refl::make_const_string('<') +                                            \
													Grafkit::Utils::Signature::FindTypeName<T>::value + refl::make_const_string(", ") +                        \
													Grafkit::Utils::Signature::FindTypeName<U>::value + refl::make_const_string('>');                          \
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
			template <typename Type> static constexpr auto CalcFieldsString()
			{
				constexpr auto members = filter(refl::type_descriptor<Type>::members, [](auto member) { return Traits::is_serializable_field(member); });
				return refl::util::accumulate(
					members,
					[](const auto accumulated, const auto member) {
						using Descriptor = decltype(member);
						constexpr auto typeName = FindTypeName<typename Descriptor::value_type>::value;
						constexpr auto name = member.name;
						return accumulated + typeName + refl::make_const_string(' ') + name + refl::make_const_string("; ");
					},
					refl::make_const_string());
			}

			template <typename Type> static constexpr auto CalcGettersString()
			{
				constexpr auto members = filter(refl::type_descriptor<Type>::members, [](auto member) { return Traits::is_serializable_getter(member); });
				return refl::util::accumulate(
					members,
					[](const auto accumulated, const auto member) {
						using Descriptor = decltype(member);
						constexpr auto typeName = FindTypeName<typename Descriptor::template return_type<Type>>::value;
						constexpr auto name = refl::descriptor::get_display_name_const(member);
						return accumulated + typeName + refl::make_const_string(' ') + name + refl::make_const_string("; ");
					},
					refl::make_const_string());
			}

		} // namespace Impl

		// TODO: Remove trailing space
		template <typename Type> static constexpr auto CalcString()
		{
			// The only wat to do this the semi-right way to process the fields nad the getters/setters separately
			return Impl::CalcFieldsString<Type>() + Impl::CalcGettersString<Type>();
		}

		namespace Impl
		{

			template <typename Type> static constexpr Checksum CalcFieldChecksum()
			{
				constexpr auto members = filter(refl::type_descriptor<Type>::members, [](auto member) { return Traits::is_serializable_field(member); });
				return refl::util::accumulate(
					members,
					[](const auto checksum, const auto member) {
						using Descriptor = decltype(member);
						constexpr auto typeName = FindTypeName<typename Descriptor::value_type>::value;
						constexpr auto name = member.name;
						return checksum ^ Checksum(typeName.data) ^ Checksum(' ') ^ Checksum(name.data) ^ Checksum("; ");
					},
					Checksum());
			}

			template <typename Type> static constexpr Checksum CalcGetterChecksum()
			{
				constexpr auto members = filter(refl::type_descriptor<Type>::members, [](auto member) { return Traits::is_serializable_getter(member); });
				return refl::util::accumulate(
					members,
					[](const auto checksum, const auto member) {
						using Descriptor = decltype(member);
						constexpr auto typeName = FindTypeName<typename Descriptor::template return_type<Type>>::value;
						constexpr auto name = refl::descriptor::get_display_name_const(member);
						return checksum ^Checksum(typeName.data) ^ Checksum(' ') ^ Checksum(name.data) ^ Checksum("; ");
					},
					Checksum());
			}
			
		} // namespace Impl

		// TODO: Remove trailing space

		template <typename Type> static constexpr Checksum CalcChecksum()
		{
			// The only wat to do this the semi-right way to process the fields nad the getters/setters separately
			return Impl::CalcFieldChecksum<Type>() ^ Impl::CalcGetterChecksum<Type>();
		}

	} // namespace Utils::Signature
} // namespace Grafkit
