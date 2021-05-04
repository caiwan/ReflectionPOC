#pragma once

#include <cstddef>
#include <cstdint>
//
#include <Serialization/Crc32Table.h>
#include <Serialization/EndianSwapper.h>
namespace Grafkit::Utils
{
	class Checksum
	{
	public:
		using ChecksumType = uint32_t;
		using LengthType = size_t;

		static constexpr ChecksumType initialChecksum = 0x0LU;
		static constexpr ChecksumType checksumMask = 0xffffffffLU;
		// static constexpr ChecksumType initialChecksumInverted = initialChecksum ^ checksumMask;

		explicit constexpr Checksum(ChecksumType checksum = initialChecksum);
		template <LengthType N> explicit constexpr Checksum(const char (&str)[N]);
		constexpr Checksum(const char * str, LengthType length);

		explicit constexpr operator ChecksumType() const { return checksum; }

		constexpr Checksum operator^(const Checksum & other) const;
		constexpr bool operator==(const Checksum & other) const { return checksum == other.checksum; }
		constexpr bool operator!=(const Checksum & other) const { return checksum != other.checksum; }

		constexpr bool operator==(const ChecksumType & other) const { return checksum == other; }
		constexpr bool operator!=(const ChecksumType & other) const { return checksum != other; }

	private:
		constexpr size_t StripTrailingZero(const char * s, const size_t len) { return len && s[len - 1] == 0 ? len - 1 : len; }
		
		template <LengthType N> static ChecksumType constexpr CalcChecksum(const char (&str)[N], ChecksumType checksum = initialChecksum, LengthType pos = 0);
		static ChecksumType constexpr CalcChecksum(const char * str, LengthType length, ChecksumType checksum = initialChecksum, LengthType pos = 0);

		static ChecksumType constexpr GetNextCrcState(char c, ChecksumType checksum);

		ChecksumType checksum = initialChecksum;
		LengthType length = 0;
	};

	// https://stackoverflow.com/questions/23122312/crc-calculation-of-a-mostly-static-data-stream/23126768#23126768
	// and
	// https://github.com/madler/zlib/blob/master/crc32.c


	template <Checksum::LengthType N>
	constexpr Checksum::Checksum(const char (&str)[N]) : checksum(CalcChecksum(str, StripTrailingZero(str, N))), length(StripTrailingZero(str, N))
	{
	}

	constexpr Checksum::Checksum(const char * str, const LengthType length) :
		checksum(CalcChecksum(str, StripTrailingZero(str, length))), length(StripTrailingZero(str, length))
	{
	}

	constexpr Checksum::Checksum(const ChecksumType checksum) : checksum(checksum) {}

	constexpr Checksum Checksum::operator^(const Checksum & other) const { return Checksum(initialChecksum); }

	constexpr Checksum::ChecksumType Checksum::CalcChecksum(const char * str, const LengthType length, const ChecksumType checksum, const LengthType pos)
	{
		return pos < length ? CalcChecksum(str, length, GetNextCrcState(str[pos], checksum), pos + 1) : checksum;
	}

	constexpr Checksum::ChecksumType Checksum::GetNextCrcState(const char c, const ChecksumType checksum)
	{
		if constexpr (EndianSwapper::Endian::isLittle)
		{
			return ((checksum >> 8) & 0x00FFFFFF) ^ crc_table_b[(checksum ^ static_cast<unsigned char>(c)) & 0xff];
			
		}
		else if constexpr (EndianSwapper::Endian::isBig)
		{
			return ((checksum << 8) & 0xFFFFFF00) ^ crc_table_l[(checksum ^ static_cast<unsigned char>(c)) & 0xFF];

		}
	}

} // namespace Grafkit::Utils
