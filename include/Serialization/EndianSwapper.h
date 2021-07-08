#pragma once

#include <cstddef>
#include <cstdint>

namespace Grafkit::EndianSwapper
{
	namespace Endian
	{
		namespace Impl // Private
		{
			static constexpr std::uint32_t little{ 0x41424344u };
			static constexpr std::uint32_t big{ 0x44434241u };
			static constexpr std::uint32_t native{ 'ABCD' };
		} // namespace Impl

		// Public
		enum class Type : size_t
		{
			Unknown,
			Little,
			Big
		};

		// Compare
		static constexpr bool isLittle = Impl::native == Impl::little;
		static constexpr bool isBig = Impl::native == Impl::big;
		static constexpr bool isUnknown = isLittle == isBig;

		// Endian type on current platform
		static constexpr Type nativeType = isLittle ? Type::Little : isBig ? Type::Big : Type::Unknown;

		static_assert(!isUnknown, "Error: Unsupported endian!");
	} // namespace Endian

	class SwapByteBase
	{
	public:
		static constexpr bool ShouldSwap() { return Endian::isBig; }

		static void SwapBytes(uint8_t & v1, uint8_t & v2)
		{
			uint8_t tmp = v1;
			v1 = v2;
			v2 = tmp;
		}
	};

	template <class T, int S> class SwapByte : public SwapByteBase
	{
	public:
		static T Swap(T v) { return v; }
	};

	template <class T> class SwapByte<T, 1> : public SwapByteBase
	{
	public:
		static T Swap(T v) { return v; }
	};

	template <class T> class SwapByte<T, 2> : public SwapByteBase
	{
	public:
		static T Swap(T v)
		{
			if constexpr (ShouldSwap()) return ((uint16_t)v >> 8) | ((uint16_t)v << 8);
			else
				return v;
		}
	};

	template <class T> class SwapByte<T, 4> : public SwapByteBase
	{
	public:
		static T Swap(T v)
		{
			if constexpr (ShouldSwap())
			{ return (SwapByte<uint16_t, 2>::Swap((uint32_t)v & 0xffff) << 16) | (SwapByte<uint16_t, 2>::Swap(((uint32_t)v & 0xffff0000) >> 16)); }
			else
				return v;
		}
	};

	template <class T> class SwapByte<T, 8> : public SwapByteBase
	{
	public:
		static T Swap(T v)
		{
			if constexpr (ShouldSwap())
				return (((uint64_t)SwapByte<uint32_t, 4>::Swap((uint32_t)(v & 0xffffffffull))) << 32) | (SwapByte<uint32_t, 4>::Swap((uint32_t)(v >> 32)));
			else
				return v;
		}
	};

	template <> class SwapByte<float, 4> : public SwapByteBase
	{
	public:
		static float Swap(const float v)
		{
			union {
				float f;
				uint8_t c[4];
			};
			f = v;
			if constexpr (ShouldSwap())
			{
				SwapBytes(c[0], c[3]);
				SwapBytes(c[1], c[2]);
			}
			else
				return f;
		}
	};

	template <> class SwapByte<double, 8> : public SwapByteBase
	{
	public:
		static double Swap(const double v)
		{
			union {
				double f;
				uint8_t c[8];
			};
			f = v;
			if constexpr (ShouldSwap())
			{
				SwapBytes(c[0], c[7]);
				SwapBytes(c[1], c[6]);
				SwapBytes(c[2], c[5]);
				SwapBytes(c[3], c[4]);
			}
			else
				return f;
		}
	};
} // namespace Grafkit::EndianSwapper
