#pragma once
namespace nemu
{
	constexpr static unsigned Bit0 = (1 << 0);
	constexpr static unsigned Bit1 = (1 << 1);
	constexpr static unsigned Bit2 = (1 << 2);
	constexpr static unsigned Bit3 = (1 << 3);
	constexpr static unsigned Bit4 = (1 << 4);
	constexpr static unsigned Bit5 = (1 << 5);
	constexpr static unsigned Bit6 = (1 << 6);
	constexpr static unsigned Bit7 = (1 << 7);
	constexpr static unsigned Bit8 = (1 << 8);

	// Determines if the nth bit is set
	bool constexpr NthBit(unsigned x, unsigned n)
	{
		return (((x) >> (n)) & 1);
	}
}