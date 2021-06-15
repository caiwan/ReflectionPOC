
#pragma once
#include <Serialization/Serialization.h>
#include <functional>
#include <map>
#include <refl.h>

namespace Grafkit
{
	class SerializerBase;
}

namespace Grafkit::Serializer
{

	class Dynamics;
	class AddFactory;

	class DynamicObject
	{
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

		typedef std::function<DynamicObject *()> FactoryFunction;

		// template <class T> static void Load(Archive & ar, T *& obj);

		// template <class T> static void Store(const Archive & ar, T * const & obj);

		// template <class T> static void Load(Archive & ar, std::shared_ptr<T> & obj);

		// template <class T> static void Store(const Archive & ar, const std::shared_ptr<T> & obj);

		static Dynamics & Instance()
		{
			static Dynamics instance;
			return instance;
		}

		DynamicObject * Create(const char * className);

	protected:
		void AddCloneable(const char * const & className, const FactoryFunction & factory) { mFactories[className] = factory; }

	private:
		Dynamics() = default;
		~Dynamics() = default;

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
} // namespace Grafkit::Serializer

#define DYNAMICS_DECL(DYNAMIC_CLASS)                                                                                                                           \
private:                                                                                                                                                       \
	static Grafkit::Serializer::Dynamics::AddFactory<DYNAMIC_CLASS> _dynamicsAddFactory;                                                                       \
	std::string_view DYNAMIC_CLASS::_DynamicsGetClazzName();                                                                                                   \
	Grafkit::Utils::Checksum _DynamicsGetClazzChecksum() override;                                                                                             \
	void _DynamicsInvokeSerializationRead(const Grafkit::SerializerBase & s) override;                                                                         \
	void _DynamicsInvokeSerializationWrite(Grafkit::SerializerBase & s) const override;

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
	void DYNAMIC_CLASS::_DynamicsInvokeSerializationRead(const Grafkit::SerializerBase & s)                                                                    \
	{                                                                                                                                                          \
		constexpr auto members = filter(refl::type_descriptor<DYNAMIC_CLASS>::members, [](auto member) { return Grafkit::Traits::is_serializable(member); });  \
		refl::util::for_each(members, [&](auto member) {                                                                                                       \
			auto & memberValue = member(*this);                                                                                                                \
			s >> memberValue;                                                                                                                                  \
		});                                                                                                                                                    \
	}                                                                                                                                                          \
                                                                                                                                                               \
	void DYNAMIC_CLASS::_DynamicsInvokeSerializationWrite(Grafkit::SerializerBase & s) const                                                                   \
	{                                                                                                                                                          \
		constexpr auto members = filter(refl::type_descriptor<DYNAMIC_CLASS>::members, [](auto member) { return Grafkit::Traits::is_serializable(member); });  \
		refl::util::for_each(members, [&](auto member) {                                                                                                       \
			const auto & memberValue = member(*this);                                                                                                                \
			s << memberValue;                                                                                                                                  \
		});                                                                                                                                                    \
	}
