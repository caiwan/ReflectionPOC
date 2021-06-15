#pragma once

#include <Serialization/Binary.h>
#include <Serialization/Json.h>

namespace Grafkit
{

	// TODO: READ = Const, WRITE = Not const
	
	class SerializerBase
	{
	public:
		template <class T> const SerializerBase & operator<<(const T & value) const { return *this; }
		template <class T> SerializerBase & operator>>(T & value) { return *this; }
	};

	template <typename Base> class SerializerMixin : public SerializerBase, public Base
	{
	public:
		using Base::Base;

		template <class T> const SerializerMixin & operator<<(const T & value) const
		{
			Base::Write(value);
			return *this;
		}

		template <class T> SerializerMixin & operator>>(T & value)
		{
			Base::Read(value);
			return *this;
		}
	};

	using BinarySerializer = SerializerMixin<Serializer::BinaryAdapter>;
	using JsonSerializer = SerializerMixin<Serializer::JsonAdapter>;

} // namespace Grafkit
