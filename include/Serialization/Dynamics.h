
#pragma once

#include <functional>
#include <map>
#include <refl.h>
//
#include <Serialization/Crc32.h>
#include <Serialization/SerializerBase.h>
#include <Serialization/Traits.h>

namespace Grafkit::Serializer
{

	class Dynamics;
	class AddFactory;

	class DynamicObject;

	class Dynamics
	{
		friend class AddFactory;

	public:
		Dynamics(const Dynamics &) = delete;
		Dynamics & operator=(const Dynamics &) = delete;
		~Dynamics() = default;

		typedef std::function<DynamicObject *()> FactoryFunction;
		template <class Serializer, class T> static void Load(const Serializer & s, T & obj);
		template <class Serializer, class T> static void Store(Serializer & s, T const & obj);

		DynamicObject * Create(const char * className) const;

		static Dynamics & Instance()
		{
			static Dynamics instance{};
			return instance;
		}

	protected:
		void AddCloneable(const char * const & className, const FactoryFunction & factory) { mFactories[className] = factory; }

	private:
		Dynamics() = default;

		std::map<std::string, FactoryFunction> mFactories;

	public:
		// Helper that adds dynamics factory in compile time
		template <class DynamicClass> class AddFactory
		{
		public:
			explicit AddFactory(const std::string_view & clazzName)
			{
				Instance().AddCloneable(clazzName.data(), []() { return new DynamicClass(); });
			}
		};
	};

#define DECL_DYNAMIC_IO_VIRTUAL_EX_(SERIALIZER_CLAZZ, ...)                                                                                                     \
	virtual void _DynamicsInvokeSerializationLoad(const SERIALIZER_CLAZZ & s) = 0;                                                                             \
	virtual void _DynamicsInvokeSerializationStore(SERIALIZER_CLAZZ & s) const = 0;                                                                            \
	__VA_ARGS__

#define DECL_DYNAMIC_IO_VIRTUAL(...) REFL_DETAIL_FOR_EACH(DECL_DYNAMIC_IO_VIRTUAL_EX_, __VA_ARGS__)

	class DynamicObject
	{
		friend class Dynamics;

	public:
		virtual ~DynamicObject() = default;

	private:
		friend class Dynamics;
		virtual std::string_view _DynamicsGetClazzName() = 0;
		virtual Utils::Checksum _DynamicsGetClazzChecksum() = 0;

		DECL_DYNAMIC_IO_VIRTUAL(GK_SERIALIZER_ADAPTER_LIST)
	};

#undef DECL_DYNAMIC_IO_VIRTUAL_EX_
#undef DECL_DYNAMIC_IO_VIRTUAL

	template <class Serializer, class T> void Dynamics::Load(const Serializer & s, T & obj)
	{
		static_assert(Traits::is_pointer_like_v<T>);

		if constexpr (Traits::is_shared_ptr_v<T> || Traits::is_unique_ptr_v<T>)
		{
			using PointerType = typename T::element_type *;
			PointerType p = nullptr;
			Load<Serializer, PointerType>(s, p);

			obj.reset(p);
		}
		else
		{

			std::string clazzName;

			s >> clazzName;

			if (clazzName.empty())
			{
				obj = nullptr;
			}
			else
			{
				DynamicObject * const dynamicObj = Instance().Create(clazzName.c_str());

				if (!dynamic_cast<T>(dynamicObj))
				{
					throw std::runtime_error("Cannot instantiate class: Given <T> is not Serializable or defined in dynamics");
				}

				Utils::Checksum::ChecksumType checksum;
				s >> checksum;

				if (checksum != dynamicObj->_DynamicsGetClazzChecksum().value())
				{
					throw std::runtime_error("Checksum mismatch");
				}

				dynamicObj->_DynamicsInvokeSerializationLoad(s);
				obj = dynamic_cast<T>(dynamicObj);
			}
		}
	}

	template <class Serializer, class T> void Dynamics::Store(Serializer & s, T const & obj)
	{
		static_assert(Traits::is_pointer_like_v<T>);

		if constexpr (Traits::is_shared_ptr_v<T> || Traits::is_unique_ptr_v<T>)
		{
			using PointerType = typename T::element_type *;
			PointerType p = obj.get();
			Store<Serializer, PointerType>(s, p);
		}
		else
		{

			if (obj == nullptr)
			{
				s << std::string();
			}
			else
			{
				DynamicObject * const dynamicObj = dynamic_cast<DynamicObject *>(obj);

				if (!dynamicObj)
				{
					throw std::runtime_error("Cannot invoke store for class: Given <T> is not Serializable or defined in dynamics");
				}

				const auto clazzName = std::string(dynamicObj->_DynamicsGetClazzName().data());
				const auto clazzChecksum = dynamicObj->_DynamicsGetClazzChecksum().value();

				s << clazzName << clazzChecksum;

				dynamicObj->_DynamicsInvokeSerializationStore(s);
			}
		}
	}

	inline DynamicObject * Dynamics::Create(const char * className) const
	{
		const auto it = mFactories.find(std::string(className));
		return it != mFactories.end() ? it->second() : nullptr;
	}

} // namespace Grafkit::Serializer

#define DECL_DYNAMIC_IO_EX_(SERIALIZER_CLAZZ, ...)                                                                                                             \
	void _DynamicsInvokeSerializationLoad(const SERIALIZER_CLAZZ & s) override { s >> *this; }                                                                 \
	void _DynamicsInvokeSerializationStore(SERIALIZER_CLAZZ & s) const override { s << *this; }                                                                \
	__VA_ARGS__

#define DECL_DYNAMIC_IO(...) REFL_DETAIL_FOR_EACH(DECL_DYNAMIC_IO_EX_, __VA_ARGS__)

#define DYNAMICS_DECL(DYNAMIC_CLASS)                                                                                                                           \
private:                                                                                                                                                       \
	static Grafkit::Serializer::Dynamics::AddFactory<DYNAMIC_CLASS> _dynamicsAddFactory;                                                                       \
	std::string_view DYNAMIC_CLASS::_DynamicsGetClazzName();                                                                                                   \
	Grafkit::Utils::Checksum _DynamicsGetClazzChecksum() override;                                                                                             \
	DECL_DYNAMIC_IO(GK_SERIALIZER_ADAPTER_LIST)

#define DYNAMICS_IMPL(DYNAMIC_CLASS)                                                                                                                           \
	Grafkit::Serializer::Dynamics::AddFactory<DYNAMIC_CLASS> DYNAMIC_CLASS## ::_dynamicsAddFactory(refl::type_descriptor<DYNAMIC_CLASS>::name.data);           \
                                                                                                                                                               \
	std::string_view DYNAMIC_CLASS::_DynamicsGetClazzName()                                                                                                    \
	{                                                                                                                                                          \
		using TypeDescriptor = refl::type_descriptor<DYNAMIC_CLASS>;                                                                                           \
		return std::string_view(TypeDescriptor::name.data);                                                                                                    \
	}                                                                                                                                                          \
                                                                                                                                                               \
	Grafkit::Utils::Checksum DYNAMIC_CLASS::_DynamicsGetClazzChecksum() { return Grafkit::Utils::Signature::CalcChecksum<DYNAMIC_CLASS>(); }
