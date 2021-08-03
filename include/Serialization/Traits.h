#pragma once

#include <functional>
#include <type_traits>
//
#include <refl.h>

namespace Grafkit
{

	// TODO These could be extracted
	namespace Attributes
	{

		using Property = refl::attr::property;

		/**
		 * Marker for fields to be serialized
		 */
		struct Serializable : refl::attr::usage::member
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
		 * https://stackoverflow.com/questions/34047732/convert-member-function-pointer-to-ordinary-function-pointer
		 * https://en.cppreference.com/w/cpp/utility/functional/mem_fn
		 * Gets the second agument of cdecl fnptr, which is the first argument to be taken after (const &this)
		 */

		// template<typename F> using FirstArgumentType = decltype(std::mem_fn(F{}))::second_argument_type;
		template <typename D> struct FirstArgumentTypeFromDescriptor
		{
			typedef typename decltype(std::mem_fn(D::pointer))::second_argument_type Type;
		};

		template <typename D> using FirstArgumentTypeFromDescriptorT = typename FirstArgumentTypeFromDescriptor<D>::Type;

		// Const-less setter-compatible type. Use this
		template <typename D> using SetterTypeFromDescriptor = std::decay_t<FirstArgumentTypeFromDescriptorT<D>>;

		/**
		 *
		 */

		//// Trait version of refl::descriptr::is_readable and is_writable
		//namespace Detail
		//{
		//	template <typename Descriptor> using IsMemberProperty = std::conjunction<refl::trait::is_member<Descriptor>, refl::trait::is_property<Descriptor>>;

		//	template <typename Descriptor> using IsReadInvocable = std::is_invocable<Descriptor, const typename Descriptor::declaring_type &>;
		//	template <typename Descriptor>
		//	using IsWriteInvocable = std::is_invocable<Descriptor, const typename Descriptor::declaring_type &, refl::descriptor::detail::placeholder>;

		//	template <typename Descriptor>
		//	using IsReturnTypeNotVoid = std::negate<std::is_void<typename Descriptor::template return_type<const typename Descriptor::declaring_type &>>>;

		//	template <typename Descriptor>
		//	using IsReturnTypeNotConst = std::negate<std::is_const<typename refl::trait::remove_qualifiers_t<Descriptor>::value_type>>;
		//} // namespace Detail

		//// readable
		//template <typename Descriptor>
		//using IsReadable = std::conjunction<Detail::IsMemberProperty<Descriptor>, Detail::IsReadInvocable<Descriptor>, Detail::IsReturnTypeNotVoid<Descriptor>>;

		//// writable
		//template <typename Descriptor>
		//using IsWritable = std::conjunction<Detail::IsMemberProperty<Descriptor>,
		//	std::disjunction<Detail::IsWriteInvocable<Descriptor>, Detail::IsReturnTypeNotConst<Descriptor>>>;

		///**
		// *
		// */

		//template <typename Descriptor, typename Attribute> using HasAttribute = refl::trait::contains_base<Attribute, typename Descriptor::attribute_types>;

		//template <typename Descriptor>
		//using IsSerializable = std::conjunction<HasAttribute<Descriptor, Attributes::Serializable>,
		//	std::disjunction<refl::trait::is_field<Descriptor>, refl::trait::is_property<Descriptor>>>;

		//// field
		//template <typename Descriptor> using IsSerializableField = std::conjunction<IsSerializable<Descriptor>, refl::trait::is_field<Descriptor>>;

		//// getter
		//template <typename Descriptor>
		//using IsSerializableGetter = std::conjunction<IsSerializable<Descriptor>, refl::trait::is_function<Descriptor>, IsReadable<Descriptor>>;

		//// setter
		//template <typename Descriptor>
		//using IsSerializableSetter = std::conjunction<IsSerializable<Descriptor>, refl::trait::is_function<Descriptor>, IsWritable<Descriptor>>;

		/**
		 *
		 */

		template <typename T> static constexpr bool is_serializable(const T & t)
		{
			return (refl::descriptor::is_field(t) || refl::descriptor::is_property(t)) && refl::descriptor::has_attribute<Attributes::Serializable>(t);
		}

		template <typename T> static constexpr bool is_serializable_field(const T & t) { return is_serializable(t) && refl::descriptor::is_field(t); }

		template <typename T> static constexpr bool is_serializable_getter(const T & t)
		{
			return is_serializable(t) && refl::descriptor::is_function(t) && refl::descriptor::is_readable(t);
		}

		template <typename T> static constexpr bool is_serializable_readable(const T & t)
		{
			return is_serializable(t) && refl::descriptor::is_readable(t); // && (is_serializable_getter(t) || is_serializable_field(t));
		}

		template <typename T> static constexpr bool is_serializable_setter(const T & t)
		{
			return is_serializable(t) && refl::descriptor::is_function(t) && refl::descriptor::is_writable(t);
		}

		template <typename T> static constexpr bool is_serializable_writable(const T & t)
		{
			return is_serializable(t) && refl::descriptor::is_writable(t); //&& (is_serializable_setter(t) || is_serializable_field(t));
		}

		/**
		 *
		 */

		constexpr size_t choiceCountMax = 99;

		template <size_t I> struct Choice : Choice<I + 1>
		{
		};

		template <> struct Choice<choiceCountMax>
		{
		};

	} // namespace Traits

} // namespace Grafkit
