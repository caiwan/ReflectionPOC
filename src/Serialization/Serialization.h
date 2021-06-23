#pragma once

#include <Serialization/SerializerBase.h>
#include <Serialization/Binary.h>
#include <Serialization/Json.h>

namespace Grafkit
{	
	using BinarySerializer = Serializer::SerializerMixin<Serializer::BinaryAdapter>;
	using JsonSerializer = Serializer::SerializerMixin<Serializer::JsonAdapter>;
} // namespace Grafkit
