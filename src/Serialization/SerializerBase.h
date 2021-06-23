#pragma once

namespace Grafkit::Serializer
{
	class SerializerBase
	{
	public:
		template <class T> SerializerBase & operator<<(const T & value) { return *this; }
		template <class T> const SerializerBase & operator>>(T & value) const { return *this; }
	};

	template <typename Base> class SerializerMixin : public SerializerBase, public Base
	{
	public:
		using Base::Base;

		template <class T> SerializerMixin & operator<<(const T & value)
		{
			Base::Write(value);
			return *this;
		}

		template <class T> const SerializerMixin & operator>>(T & value) const
		{
			Base::Read(value);
			return *this;
		}
	};
}
