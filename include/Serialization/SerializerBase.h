#pragma once

namespace Grafkit
{
	namespace Serializer
	{

		class BinaryAdapter;
		class JsonAdapter;

		class SerializerBase
		{
		public:
			template <class T> SerializerBase & operator<<(const T & value) { return *this; }
			template <class T> const SerializerBase & operator>>(T & value) const { return *this; }
		};

	} // namespace Serializer
} // namespace Grafkit

#define GK_SERIALIZER_ADAPTER_LIST Grafkit::Serializer::BinaryAdapter, Grafkit::Serializer::JsonAdapter
