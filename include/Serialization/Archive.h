#pragma once

#if 0

// Based on
// http://www.codeproject.com/Tips/495191/Serialization-implementation-in-Cplusplus
// and https://github.com/voidah/archive/blob/master/archive.h
// TODO: Naming has noting to do with the expected behaviour behind 'clone', so
// please rename it later on

#include <algorithm>
#include <cassert>
#include <deque>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <type_traits>
#include <vector>

//#include <grafkit/common.h>
//#include <grafkit/utils/stringutils.h>
#include <Serialization/EndianSwapper.h>
#include <Serialization/Persistence.h>
#include <Serialization/Stream.h>

// https://stackoverflow.com/questions/41853159/how-to-detect-if-a-type-is-shared-ptr-at-compile-time
template <typename T> struct is_shared_ptr : std::false_type
{
};

template <typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type
{
};

template <typename T> constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;

template <typename T> struct is_unique_ptr : std::false_type
{
};

template <typename T> struct is_unique_ptr<std::unique_ptr<T>> : std::true_type
{
};

template <typename T> constexpr bool is_unique_ptr_v = is_shared_ptr<T>::value;

namespace Grafkit
{
	class IStream;
	class ISerializable;
	class Archive;
	class Persistence;

	class AddFactory;

	namespace Traits
	{

		template <typename T> struct is_storing : std::is_same<T, const Archive &>
		{
		};

		template <typename T> constexpr bool is_storing_v = is_storing<T>::value;

		template <typename T> struct is_loading : std::is_same<T, Archive &>
		{
		};

		template <typename T> constexpr bool is_loading_v = is_loading<T>::value;

	}

	class Archive
	{
	public:
		// ---------------------------------------------------------------------------------------------------------------------------------

		explicit Archive(std::unique_ptr<IStream> & stream) : mStream(move(stream)) {}
		explicit Archive(std::unique_ptr<IStream> && stream) : mStream(move(stream)) {}

		Archive(const Archive & other) = delete;
		Archive & operator=(const Archive & other) = delete;

		// ---
		template <class T> const inline Archive & operator<<(const T & v) const
		{
			*this & v;
			return *this;
		}

		template <class T> inline Archive & operator>>(T & v)
		{
			*this & v;
			return *this;
		}

		// Read
		template <typename T> Archive & operator&(T & v)
		{
			if constexpr (std::is_enum_v<T>)
			{
				int iv = 0;
				*this & iv;
				v = T(iv);
			}
			else if constexpr (std::is_pointer_v<T>)
			{
				Persistence::Load(*this, v);
			}
			else if constexpr (is_shared_ptr_v<T>)
			{
				Persistence::Load(*this, v);
			}
			else if constexpr (is_unique_ptr_v<T>)
			{
				T * p = nullptr;
				Persistence::Load(*this, p);
				v = std::unique_ptr<T>(p);
			}
			else
			{
				v.Serialize(*this);
			}

			return *this;
		}

		// Write
		template <typename T> const Archive & operator&(const T & v) const
		{
			if constexpr (std::is_enum_v<T>)
			{
				const int iv = int(v);
				void(*this & iv);
			}
			else if constexpr (std::is_pointer_v<T>)
			{
				Persistence::Store(*this, const_cast<T &>(v));
			}
			else if constexpr (is_shared_ptr_v<T>)
			{
				Persistence::Store(*this, v);
			}
			else if constexpr (is_unique_ptr_v<T>)
			{
				Persistence::Store(*this, v);
			}
			else
			{
				const_cast<T &>(v).Serialize(*this);
			}

			return *this;
		}

		template <class T, size_t N> Archive & operator&(T (&v)[N])
		{
			uint32_t len = 0;
			*this & len;
			for (size_t i = 0; i < N; ++i) *this & v[i];
			return *this;
		}

		template <class T, size_t N> const Archive & operator&(const T (&v)[N]) const
		{
			const uint32_t length = N;
			void(*this & length);
			for (size_t i = 0; i < N; ++i) *this & v[i];
			return *this;
		}

		template <class T, size_t N> Archive & operator&(std::array<T, N> & v)
		{
			uint32_t length = 0;
			void(*this & length);
			for (size_t i = 0; i < N; ++i) *this & v[i];
			return *this;
		}

		template <class T, size_t N> const Archive & operator&(const std::array<T, N> & v) const
		{
			const uint32_t length = N;
			void(*this & length);
			for (size_t i = 0; i < N; ++i) *this & v[i];
			return *this;
		}

#define SERIALIZER_FOR_POD(type)                                                                                                                               \
	Archive & operator&(type & v)                                                                                                                              \
	{                                                                                                                                                          \
		const size_t lastPos = static_cast<std::istream &>(*(mStream.get())).tellg();                                                                          \
		mStream->Read((char *)&v, sizeof(type));                                                                                                               \
		if (!mStream->IsSuccess()) throw std::runtime_error("malformed data - lastPos: " + std::to_string(lastPos));                                           \
		v = Swap(v);                                                                                                                                           \
		return *this;                                                                                                                                          \
	}                                                                                                                                                          \
                                                                                                                                                               \
	const Archive & operator&(const type v) const                                                                                                              \
	{                                                                                                                                                          \
		const size_t lastPos = static_cast<std::ostream &>(*(mStream.get())).tellp();                                                                          \
		const type v2 = Swap(v);                                                                                                                               \
		mStream->Write((const char *)&v2, sizeof(type));                                                                                                       \
		return *this;                                                                                                                                          \
	}

		SERIALIZER_FOR_POD(bool);
		SERIALIZER_FOR_POD(char);
		SERIALIZER_FOR_POD(unsigned char);
		SERIALIZER_FOR_POD(short);
		SERIALIZER_FOR_POD(unsigned short);
		SERIALIZER_FOR_POD(int);
		SERIALIZER_FOR_POD(unsigned int);
		SERIALIZER_FOR_POD(long);
		SERIALIZER_FOR_POD(unsigned long);
		SERIALIZER_FOR_POD(long long);
		SERIALIZER_FOR_POD(unsigned long long);
		SERIALIZER_FOR_POD(float);
		SERIALIZER_FOR_POD(double);

#define SERIALIZER_FOR_STL_CONTAINER(type)                                                                                                                     \
	template <class T> Archive & operator&(type<T> & v)                                                                                                        \
	{                                                                                                                                                          \
		uint32_t len = 0;                                                                                                                                      \
		*this & len;                                                                                                                                           \
		v.clear();                                                                                                                                             \
		for (uint32_t i = 0; i < len; ++i)                                                                                                                     \
		{                                                                                                                                                      \
			T value;                                                                                                                                           \
			*this & value;                                                                                                                                     \
			v.insert(v.end(), value);                                                                                                                          \
		}                                                                                                                                                      \
		return *this;                                                                                                                                          \
	}                                                                                                                                                          \
                                                                                                                                                               \
	template <class T> const Archive & operator&(const type<T> & v) const                                                                                      \
	{                                                                                                                                                          \
		const uint32_t len = static_cast<uint32_t>(v.size());                                                                                                  \
		void(*this & len);                                                                                                                                     \
		for (typename type<T>::const_iterator it = v.begin(); it != v.end(); ++it) *this &* it;                                                                \
		return *this;                                                                                                                                          \
	}

#define SERIALIZER_FOR_STL_MAP(type)                                                                                                                           \
	template <class T1, class T2> Archive & operator&(type<T1, T2> & v)                                                                                        \
	{                                                                                                                                                          \
		uint32_t len = 0;                                                                                                                                      \
		*this & len;                                                                                                                                           \
		v.clear();                                                                                                                                             \
		for (uint32_t i = 0; i < len; ++i)                                                                                                                     \
		{                                                                                                                                                      \
			std::pair<T1, T2> value;                                                                                                                           \
			*this & value;                                                                                                                                     \
			v.insert(v.end(), value);                                                                                                                          \
		}                                                                                                                                                      \
		return *this;                                                                                                                                          \
	}                                                                                                                                                          \
                                                                                                                                                               \
	template <class T1, class T2> const Archive & operator&(const type<T1, T2> & v) const                                                                      \
	{                                                                                                                                                          \
		const uint32_t len = static_cast<uint32_t>(v.size());                                                                                                  \
		void(*this & len);                                                                                                                                     \
		for (typename type<T1, T2>::const_iterator it = v.begin(); it != v.end(); ++it) *this &* it;                                                           \
		return *this;                                                                                                                                          \
	}

		SERIALIZER_FOR_STL_CONTAINER(std::vector);
		SERIALIZER_FOR_STL_CONTAINER(std::deque);
		SERIALIZER_FOR_STL_CONTAINER(std::list);
		SERIALIZER_FOR_STL_CONTAINER(std::set);
		SERIALIZER_FOR_STL_CONTAINER(std::multiset);
		SERIALIZER_FOR_STL_MAP(std::map);
		SERIALIZER_FOR_STL_MAP(std::multimap);
		SERIALIZER_FOR_STL_MAP(std::unordered_map);
		SERIALIZER_FOR_STL_MAP(std::unordered_multimap);

		template <class T1, class T2> Archive & operator&(std::pair<T1, T2> & v)
		{
			*this & v.first & v.second;
			return *this;
		}

		template <class T1, class T2> const Archive & operator&(const std::pair<T1, T2> & v) const
		{
			*this & v.first & v.second;
			return *this;
		}

		Archive & operator&(std::string & v)
		{
			uint32_t strLength = 0;
			*this & strLength;
			v.clear();
			char buffer[4096];
			uint32_t toRead = strLength;
			while (toRead != 0)
			{
				// A Quick and dirty min had to be introduced here
				// in form of this immediately invoked lambda.
				// I'm sorry about that.
				const uint32_t readLength = [](const auto a, const auto b) { return a < b ? a : b; }(toRead, uint32_t(sizeof(buffer)));
				const size_t lastPos = static_cast<std::istream &>(*(mStream.get())).tellg();
				mStream->Read(buffer, readLength);
				if (!mStream->IsSuccess()) throw std::runtime_error("malformed data - lastPos: " + std::to_string(lastPos));
				v += std::string(buffer, readLength);
				toRead -= readLength;
			}
			v.pop_back(); // rm last trailing
			return *this;
		}

		const Archive & operator&(const std::string & v) const
		{
			const size_t lastPos = static_cast<std::ostream &>(*(mStream.get())).tellp();
			const auto length = uint32_t(v.length() + 1);
			void(*this & length);
			mStream->Write(v.c_str(), length);
			return *this;
		}

	private:
		template <class T> T Swap(const T & v) const { return EndianSwapper::SwapByte<T, sizeof(T)>::Swap(v); }

		// ---
		std::unique_ptr<IStream> mStream;
	};

} // namespace Grafkit

// ----

// TODO:
// https://stackoverflow.com/questions/7582546/using-generic-stdfunction-objects-with-member-functions-in-one-class
// TODO: Merge macros later

#define DYNAMICS_DECL(CLAZZ)                                                                                                                                   \
private:                                                                                                                                                       \
	static Grafkit::Persistence::AddFactory<CLAZZ> __addDynamicsFactory;

#define DYNAMICS_IMPL(CLAZZ) Grafkit::Persistence::AddFactory<CLAZZ> CLAZZ## ::__addDynamicsFactory(#CLAZZ)

#define PERSISTENT_INVOKE_SERIALIZE()                                                                                                                          \
	void _InvokeSerialize(const Grafkit::Archive & ar) override { Serialize(ar); }                                                                             \
	void _InvokeDeserialize(Grafkit::Archive & ar) override { Serialize(ar); }

#define PERSISTENT_DECL(CLAZZ, VERSION)                                                                                                                        \
public:                                                                                                                                                        \
	std::string GetClazzName() const override { return #CLAZZ; }                                                                                               \
	uint16_t GetVersion() const override { return VERSION; }                                                                                                   \
                                                                                                                                                               \
protected:                                                                                                                                                     \
	PERSISTENT_INVOKE_SERIALIZE()                                                                                                                              \
	DYNAMICS_DECL(CLAZZ)

// --
#define SERIALIZE(CLAZZ, VERSION, myAr)                                                                                                                        \
	PERSISTENT_DECL(CLAZZ, VERSION)                                                                                                                            \
public:                                                                                                                                                        \
	template <class ArchiveType> void Serialize(ArchiveType & myAr)

#define PERSISTENT_IMPL(CLAZZ) DYNAMICS_IMPL(CLAZZ)

// ----
// -- QnD Hack time
// Template specialization

// TODO FIXME: Use proper type id naming, etc. not this abomination
#define PERSISTENT_TEMPLATE_DECL(CLAZZ, TYPE, VERSION)                                                                                                         \
public:                                                                                                                                                        \
	std::string GetClazzName() const override                                                                                                                  \
	{                                                                                                                                                          \
		return Grafkit::Utils::StringReplace(                                                                                                                  \
		    std::string(#CLAZZ), std::string("<"## #TYPE##">"), std::string("<") + std::string(Utils::TypeName<TYPE>()) + std::string(">"));                   \
	}                                                                                                                                                          \
	uint16_t GetVersion() const override { return VERSION; };                                                                                                  \
                                                                                                                                                               \
protected:                                                                                                                                                     \
	PERSISTENT_INVOKE_SERIALIZE()

// --

#define SERIALIZE_TEMPLATE(CLAZZ, TYPE, VERSION, myAr)                                                                                                         \
	PERSISTENT_TEMPLATE_DECL(CLAZZ, TYPE, VERSION)                                                                                                             \
	DYNAMICS_DECL(CLAZZ)                                                                                                                                       \
public:                                                                                                                                                        \
	template <class _ArchiveType> void Serialize(_ArchiveType & myAr)

#define PERSISTENT_TEMPLATE_SPECIALIZATION(CLAZZ) Grafkit::Persistence::AddFactory<CLAZZ> CLAZZ## ::__addDynamicsFactory(#CLAZZ)

#endif
