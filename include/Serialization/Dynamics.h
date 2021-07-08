
#pragma once

#include <functional>
#include <map>
#include <refl.h>
//
#include <Serialization/Crc32.h>
#include <Serialization/SerializerBase.h>
#include <Serialization/Traits.h>

namespace Grafkit
{
	class Archive;
	class SerializerBase;

	namespace Traits
	{
#ifndef GK_MSVC_SFINAE_HAS_FN_WORKAROUND
		/**
		 * Has push_back
		 * @tparam T
		 */

		template <typename T, typename = void> struct has_dynamics_invoke_serialization_read_t : std::false_type
		{
		};

		template <typename T>
		struct has_dynamics_invoke_serialization_read_t<T,
			std::void_t<decltype(static_cast<void (T::*)(const SerializerBase &)>(&T::_DynamicsInvokeSerializationRead))>> : std::true_type
		{
		};

#else
		GK_MSVC_SFINAE_HAS_FN_WORKAROUND(_DynamicsInvokeSerializationRead);
		// To be able to befriend
		template <typename T> struct has_dynamics_invoke_serialization_read_t : test_has__DynamicsInvokeSerializationRead<T, void(const SerializerBase &)>
		{
		};

#endif
		template <typename T> using has_dynamics_invoke_serialization_read = has_dynamics_invoke_serialization_read_t<T>;
		template <typename T> constexpr bool has_dynamics_invoke_serialization_read_v = has_dynamics_invoke_serialization_read<T>::value;

#ifndef GK_MSVC_SFINAE_HAS_FN_WORKAROUND
		/**
		 * Has push_back
		 * @tparam T
		 */

		template <typename T, typename = void> struct has_dynamics_invoke_serialization_write : std::false_type
		{
		};

		template <typename T>
		struct has_dynamics_invoke_serialization_write<T,
			std::void_t<decltype(static_cast<void (T::*)(SerializerBase &)>(&T::_DynamicsInvokeSerializationWrite))>> : std::true_type
		{
		};

#else
		GK_MSVC_SFINAE_HAS_FN_WORKAROUND(_DynamicsInvokeSerializationWrite);

		// To be able to befriend
		template <typename T> struct has_dynamics_invoke_serialization_write_t : test_has__DynamicsInvokeSerializationWrite<T, void(SerializerBase &)>
		{
		};

#endif

		template <typename T> using has_dynamics_invoke_serialization_write = has_dynamics_invoke_serialization_write_t<T>;
		template <typename T> constexpr bool has_dynamics_invoke_serialization_write_v = has_dynamics_invoke_serialization_write<T>::value;

	} // namespace Traits

} // namespace Grafkit

namespace Grafkit::Serializer
{

	class Dynamics;
	class AddFactory;

	class DynamicObject
	{
		friend class Dynamics;
		template <typename T> friend struct Traits::has_dynamics_invoke_serialization_write_t;
		template <typename T> friend struct Traits::has_dynamics_invoke_serialization_read_t;

	public:
		virtual ~DynamicObject() = default;

	private:
		friend class Dynamics;
		virtual std::string_view _DynamicsGetClazzName() = 0;
		virtual Utils::Checksum _DynamicsGetClazzChecksum() = 0;
		virtual void _DynamicsInvokeSerializationRead(const SerializerBase & s) = 0;
		virtual void _DynamicsInvokeSerializationWrite(SerializerBase & s) const = 0;
	};

	class Dynamics
	{
		friend class AddFactory;

	public:
		Dynamics(const Dynamics &) = delete;
		Dynamics & operator=(const Dynamics &) = delete;
		~Dynamics() = default;

		typedef std::function<DynamicObject *()> FactoryFunction;
		template <class T> static void Load(SerializerBase & s, T *& obj);
		template <class T> static void Store(const SerializerBase & s, T * const & obj);

		template <class T> static void Load(SerializerBase & ar, std::shared_ptr<T> & obj)
		{
			T * pT = nullptr;
			Load(ar, pT);
			obj = pT;
		}
		template <class T> static void Store(const SerializerBase & ar, const std::shared_ptr<T> & obj) { Store(ar, obj.get()); }

		template <class T> static void Load(SerializerBase & ar, std::unique_ptr<T> & obj)
		{
			T * pT = nullptr;
			Load(ar, pT);
			obj = pT;
		}
		template <class T> static void Store(const SerializerBase & ar, const std::unique_ptr<T> & obj) { Store(ar, obj.get()); }

		static Dynamics & Instance()
		{
			static Dynamics instance;
			return instance;
		}

		DynamicObject * Create(const char * className) const;

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

	template <class T> void Dynamics::Load(SerializerBase & s, T *& obj)
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
			T * const typedObj = dynamic_cast<T *>(dynamicObj);

			if (!typedObj || !dynamicObj)
			{
				throw std::runtime_error("Cannot instantiate class: Given <T> is not Serializable or defined in dynamics");
			}

			Utils::Checksum::ChecksumType checksum;
			s >> checksum;

			if (checksum != typedObj->obj->_DynamicsGetClazzChecksum().value())
			{
				throw std::runtime_error("Checksum mismatch");
			}

			// static_assert(Traits::has_dynamics_invoke_serialization_read_v<T>);
			typedObj->_DynamicsInvokeSerializationRead(s);
			obj = typedObj;
		}
	}

	template <class T> void Dynamics::Store(const SerializerBase & s, T * const & obj)
	{
		if (obj == nullptr)
		{
			s << 0;
		}
		else
		{
			// static_assert(Traits::has_dynamics_invoke_serialization_write_v<T>);

			const auto clazzName = obj->_DynamicsGetClazzName();
			const auto clazzChecksum = obj->_DynamicsGetClazzChecksum().value();

			s << clazzName << clazzChecksum;

			obj->_DynamicsInvokeSerializationWrite(s, obj);
		}
	}

	inline DynamicObject * Dynamics::Create(const char * className) const
	{
		const auto it = mFactories.find(std::string(className));
		return it != mFactories.end() ? it->second() : nullptr;
	}
} // namespace Grafkit::Serializer

#define DYNAMICS_DECL(DYNAMIC_CLASS)                                                                                                                           \
private:                                                                                                                                                       \
	static Grafkit::Serializer::Dynamics::AddFactory<DYNAMIC_CLASS> _dynamicsAddFactory;                                                                       \
	std::string_view DYNAMIC_CLASS::_DynamicsGetClazzName();                                                                                                   \
	Grafkit::Utils::Checksum _DynamicsGetClazzChecksum() override;                                                                                             \
	void _DynamicsInvokeSerializationRead(const Grafkit::Serializer::SerializerBase & s) override;                                                             \
	void _DynamicsInvokeSerializationWrite(Grafkit::Serializer::SerializerBase & s) const override;

#define DYNAMICS_IMPL(DYNAMIC_CLASS)                                                                                                                           \
	Grafkit::Serializer::Dynamics::AddFactory<DYNAMIC_CLASS> DYNAMIC_CLASS## ::_dynamicsAddFactory(refl::type_descriptor<DYNAMIC_CLASS>::name.data);           \
                                                                                                                                                               \
	std::string_view DYNAMIC_CLASS::_DynamicsGetClazzName()                                                                                                    \
	{                                                                                                                                                          \
		using TypeDescriptor = refl::type_descriptor<DYNAMIC_CLASS>;                                                                                           \
		return std::string_view(TypeDescriptor::name.data);                                                                                                    \
	}                                                                                                                                                          \
                                                                                                                                                               \
	Grafkit::Utils::Checksum DYNAMIC_CLASS::_DynamicsGetClazzChecksum() { return Grafkit::Utils::Signature::CalcChecksum<DYNAMIC_CLASS>(); }                   \
                                                                                                                                                               \
	void DYNAMIC_CLASS::_DynamicsInvokeSerializationRead(const Grafkit::Serializer::SerializerBase & s) { s >> *this; }                                        \
                                                                                                                                                               \
	void DYNAMIC_CLASS::_DynamicsInvokeSerializationWrite(Grafkit::Serializer::SerializerBase & s) const { s << *this; }
