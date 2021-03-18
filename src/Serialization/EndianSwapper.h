#pragma once

#include <cstddef>
#include <cstdint>

namespace Grafkit::EndianSwapper
{
	class SwapByteBase
	{
	public:
		// TODO Can this be constexpr?
		static inline bool ShouldSwap()
		{
			static const uint16_t swapTest = 1;
			return (*((char *)&swapTest) == 1);
		}

		static inline void SwapBytes(uint8_t & v1, uint8_t & v2)
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
			if (ShouldSwap()) return ((uint16_t)v >> 8) | ((uint16_t)v << 8);
			return v;
		}
	};

	template <class T> class SwapByte<T, 4> : public SwapByteBase
	{
	public:
		static T Swap(T v)
		{
			if (ShouldSwap())
			{ return (SwapByte<uint16_t, 2>::Swap((uint32_t)v & 0xffff) << 16) | (SwapByte<uint16_t, 2>::Swap(((uint32_t)v & 0xffff0000) >> 16)); }
			return v;
		}
	};

	template <class T> class SwapByte<T, 8> : public SwapByteBase
	{
	public:
		static T Swap(T v)
		{
			if (ShouldSwap())
				return (((uint64_t)SwapByte<uint32_t, 4>::Swap((uint32_t)(v & 0xffffffffull))) << 32) | (SwapByte<uint32_t, 4>::Swap((uint32_t)(v >> 32)));
			return v;
		}
	};

	template <> class SwapByte<float, 4> : public SwapByteBase
	{
	public:
		static float Swap(float v)
		{
			union {
				float f;
				uint8_t c[4];
			};
			f = v;
			if (ShouldSwap())
			{
				SwapBytes(c[0], c[3]);
				SwapBytes(c[1], c[2]);
			}
			return f;
		}
	};

	template <> class SwapByte<double, 8> : public SwapByteBase
	{
	public:
		static double Swap(double v)
		{
			union {
				double f;
				uint8_t c[8];
			};
			f = v;
			if (ShouldSwap())
			{
				SwapBytes(c[0], c[7]);
				SwapBytes(c[1], c[6]);
				SwapBytes(c[2], c[5]);
				SwapBytes(c[3], c[4]);
			}
			return f;
		}
	};
} // namespace Grafkit::EndianSwapper
