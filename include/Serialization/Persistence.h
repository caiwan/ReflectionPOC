#pragma once

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

namespace Grafkit
{

	class ISerializable;
	class Archive;

	class Persistence
	{
		friend class AddFactory;

	public:
		Persistence(const Persistence &) = delete;
		Persistence & operator=(const Persistence &) = delete;
		~Persistence() = default;

	private:
		Persistence() = default;

	public:
		typedef std::function<ISerializable *()> FactoryFunction;

		template <class T> static void Load(Archive & ar, T *& obj);
		template <class T> static void Store(const Archive & ar, T * const & obj);

		template <class T> static void Load(Archive & ar, std::shared_ptr<T> & obj);
		template <class T> static void Store(const Archive & ar, const std::shared_ptr<T> & obj);

		static Persistence & Instance()
		{
			static Persistence instance;
			return instance;
		}

		ISerializable * Create(std::string className);

	protected:
		void AddClonable(const std::string & className, const FactoryFunction & factory) { m_factories[className] = factory; }

		// TODO Unordered map maybe?
		std::map<std::string, FactoryFunction> m_factories;

	public:
		// Helper that adds dynamics factory in compile time
		template <class CLAZZ> class AddFactory
		{
		public:
			explicit AddFactory(const char * clazzName)
			{
				Instance().AddClonable(clazzName, FactoryFunction([]() { return new CLAZZ(); }));
			}
		};
	};

	class ISerializable
	{
		friend class Persistence;

	public:
		// ---
		ISerializable() = default;
		virtual ~ISerializable() = default;

		// ---
		[[nodiscard]] virtual std::string GetClazzName() const = 0; // collision w/ Winapi macro
		[[nodiscard]] virtual uint16_t GetVersion() const = 0;

	protected:
		virtual void _InvokeSerialize(const Archive &) { throw std::runtime_error("Not implemented"); };
		virtual void _InvokeDeserialize(Archive &) { throw std::runtime_error("Not implemented"); };
	};

} // namespace Grafkit

// TODO: Move to .inl && needs some more cleaning || bring dynamics and
// persistence back
inline Grafkit::ISerializable * Grafkit::Persistence::Create(const std::string className)
{
	const auto it = m_factories.find(className);
	if (it == m_factories.end()) return nullptr;
	ISerializable * serializable = it->second();
	return serializable;
}

template <class T> void Grafkit::Persistence::Load(Archive & ar, T *& obj)
{
	assert(!obj);
	std::string clazzName;
	uint16_t version = 0;
	ar >> clazzName >> version;
	if (clazzName.empty())
	{
		obj = nullptr;
		return;
	}
	ISerializable * const instance = (Instance().Create(clazzName));
	T * dynamics = dynamic_cast<T *>(instance);
	if (!instance || !dynamics) throw std::runtime_error("Cannot instantiate class: `" + clazzName + "` is not ISerializable or defined for dynamics");
	if (version != instance->GetVersion()) throw std::runtime_error("Version mismatch");
	instance->_InvokeDeserialize(ar);
	obj = dynamics;
}

template <class T> void Grafkit::Persistence::Store(const Archive & ar, T * const & obj)
{
	// indicate that ptr is null w/ an empty string and zero
	if (!obj)
	{
		ar << "" << uint16_t(0);
		return;
	}
	auto * dynamics = dynamic_cast<ISerializable *>(obj);
	const std::string clazzName(dynamics->GetClazzName());
	const uint16_t version = dynamics->GetVersion();
	void(ar << clazzName << version);
	dynamics->_InvokeSerialize(ar);
}

template <class T> void Grafkit::Persistence::Load(Archive & ar, std::shared_ptr<T> & obj)
{
	T * ptr = nullptr;
	Load(ar, ptr);
	obj = std::shared_ptr<T>(ptr);
}

template <class T> void Grafkit::Persistence::Store(const Archive & ar, const std::shared_ptr<T> & obj)
{
	T * ptr = obj.get();
	Store(ar, ptr);
}
