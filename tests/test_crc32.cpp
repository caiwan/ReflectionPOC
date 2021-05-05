#include <Serialization/Crc32.h>
#include <gtest/gtest.h>

// This tests if Crc32 could be compiled as constexpr

// see the crc32.ipynb
constexpr uint32_t crcZero = 0x0ul;
constexpr uint32_t crcHelloWorld = 2499634191UL;

static_assert(Grafkit::Utils::Checksum() == crcZero);
static_assert(Grafkit::Utils::Checksum("\0") == crcZero);
static_assert(Grafkit::Utils::Checksum("\0\0") == crcZero);
static_assert(Grafkit::Utils::Checksum("\0\0\0") == crcZero);

// Reference CRC

TEST(Checksum, Checksum)
{
	ASSERT_EQ(uint32_t(Grafkit::Utils::Checksum()), crcZero);
	ASSERT_EQ(uint32_t(Grafkit::Utils::Checksum("\0")), crcZero);
	ASSERT_EQ(uint32_t(Grafkit::Utils::Checksum("HelloWorld")), crcHelloWorld);
}

TEST(Checksum, Concat)
{
	const auto a = "Hello";
	const auto b = "World";
	const auto crcA = Grafkit::Utils::Checksum(a, 5);
	const auto crcB = Grafkit::Utils::Checksum(b, 5);
	const auto crcAB = crcA ^ crcB;
	ASSERT_EQ(uint32_t(crcAB), crcHelloWorld);
}
