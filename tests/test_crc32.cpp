#include <Serialization/Crc32.h>

// This tests if Crc32 could be compiled as constexpr

static_assert("Hello"_crc32 == Grafkit::Utils::Crc32<'H', 'e', 'l', 'l', 'o'>::value, "CRC32 values don't match");
static_assert("0"_crc32 == Grafkit::Utils::Crc32<'0'>::value, "CRC32 values don't match");
static_assert(""_crc32 == Grafkit::Utils::Crc32<>::value, "CRC32 values don't match");

