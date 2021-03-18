#pragma once

#include<Serialization/Binary.h>
#include<Serialization/Json.h>

namespace Grafkit
{

	template <typename Base> class SerializerMixin : public Base
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
