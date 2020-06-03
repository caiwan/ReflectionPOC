#pragma once

#include <string>
#include <type_traits>
#include <vector>

// Implementation Based on
// https://preshing.com/20180116/a-primitive-reflection-system-in-cpp-part-1/
// https://preshing.com/20180116/a-primitive-reflection-system-in-cpp-part-2/
// https://medium.com/@vesko.karaganev/compile-time-reflection-in-c-17-55c14ee8106b

namespace Reflect
{

	//--------------------------------------------------------
	// Helper Classes
	//--------------------------------------------------------

	template <typename T, size_t N> struct ConstArray
	{
		constexpr ConstArray() = default;

		constexpr ConstArray(const std::initializer_list<T> & initializerList) { static_assert(initializerList.size() <= N, "ConstArray overflow"); }

		T data[N] = {};
		size_t size = N;
	};

	//--------------------------------------------------------
	// Type descriptors
	//--------------------------------------------------------

	struct TypeDescriptor
	{
		constexpr TypeDescriptor(const char * name, const size_t size) : name{ name }, size{ size } {}

		[[nodiscard]] std::string GetFullName() const { return name; }

		const char * name;
		const size_t size;
	};

	// Declare the function template that handles primitive types such as int, std::string, etc.:
	template <typename T> TypeDescriptor & GetPrimitiveDescriptor();

	/*
	 * Helper class to find type descriptors
	 */
	struct TypeResolverLookup
	{
		template <typename T> static char DescriptionFunc(decltype(&T::Reflection));
		template <typename T> static int DescriptionFunc(...);
		template <typename T> struct IsReflected
		{
			enum
			{
				Value = (sizeof(DescriptionFunc<T>(nullptr)) == sizeof(char))
			};
		};

		// Access static member when defined
		template <typename T, typename std::enable_if<IsReflected<T>::Value, int>::type = 0> static TypeDescriptor * Descriptor() { return &T::Reflection(); }

		// Call from primitive lookup-table otherwise:
		template <typename T, typename std::enable_if<!IsReflected<T>::Value, int>::type = 0> static TypeDescriptor * Descriptor()
		{
			return GetPrimitiveDescriptor<T>();
		}
	};

	// This is the primary class template for finding all TypeDescriptors:
	template <typename T> struct TypeResolver
	{
		static TypeDescriptor * Descriptor() { return TypeResolverLookup::Descriptor<T>(); }
	};

	//--------------------------------------------------------
	// Type descriptors for user-defined structs/classes
	//--------------------------------------------------------

	struct StructTypeDescriptor : TypeDescriptor
	{
		struct Member
		{
			const char * name;
			size_t offset;
			TypeDescriptor * type;
		};

		static constexpr size_t memberMax = 256;
		ConstArray<Member, memberMax> members;

		// constexpr StructTypeDescriptor(void (*init)(StructTypeDescriptor *)) : TypeDescriptor{ nullptr, 0 } { init(this); }
		constexpr StructTypeDescriptor(const char * name, size_t size, const std::initializer_list<Member> & init) :
		    TypeDescriptor{ nullptr, 0 }, members{ init }
		{
		}

		// void Dump(const void * obj, int indentLevel) const override;
	};
}; // namespace Reflect

	// inline void Reflect::StructTypeDescriptor::Dump(const void * obj, int indentLevel) const
	//{
	//	// std::cout << name << " {" << std::endl;
	//	// for (const Member & member : members)
	//	//{
	//	//	std::cout << std::string(4 * (indentLevel + 1), ' ') << member.name << " = ";
	//	//	member.type->dump((char *)obj + member.offset, indentLevel + 1);
	//	//	std::cout << std::endl;
	//	//}
	//	// std::cout << std::string(4 * indentLevel, ' ') << "}";
	//}

#define REFLECT(Descriptor)                                                                                                                                    \
	friend struct Reflect::TypeResolverLookup;                                                                                                                 \
	static Reflect::##Descriptor Reflection;                                                                                                                   \
	static void InitReflection(Reflect::##Descriptor *)

#define REFLECT_STRUCT()      REFLECT(Struct)
#define REFLECT_BEGIN_CLASS() REFLECT_BEGIN(Class)

#define REFLECT_STRUCT_BEGIN(type)                                                                                                                             \
	Reflect::StructTypeDescriptor type::Reflection{ type::InitReflection };                                                                                    \
	void type::InitReflection(Reflect::StructTypeDescriptor * typeDesc)                                                                                        \
	{                                                                                                                                                          \
		using T = type;                                                                                                                                        \
		typeDesc->name = #type;                                                                                                                                \
		typeDesc->size = sizeof(T);                                                                                                                            \
		typeDesc->members = {

#define REFLECT_FIELD(name) { #name, offsetof(T, name), reflect::TypeResolver<decltype(T::name)>::get() },

#define REFLECT_END()                                                                                                                                          \
	}                                                                                                                                                          \
	;                                                                                                                                                          \
	}
