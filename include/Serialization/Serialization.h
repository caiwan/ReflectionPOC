#pragma once

#include <Serialization/Binary.h>
#include <Serialization/Json.h>

namespace Grafkit
{
	using BinarySerializer = Serializer::BinaryAdapter;
	using JsonSerializer = Serializer::JsonAdapter;

} // namespace Grafkit
