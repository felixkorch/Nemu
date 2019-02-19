// ---------------------------------------------------------------------* C++ *-
// Memory.h
//
// -----------------------------------------------------------------------------

#pragma once

#include <array>
#include <cstdint>

namespace nemu
{
/// \class: Memory
///
/// @Cell: Internal storage type. Requires implicit std::uint8_t conversion.
///
/// The NES provides memory for 16-bit addresses up to at least 0x401F (might
/// even guarantee 0xFFFF, I haven't looked into cartrigdes). Allot of the
/// addresses are however just mirrors of others depending on the range. To
/// simulate this arrays of memory are never directly accesed but instead
/// adjusted to the mirrors when written or read.
///
/// The Cell class can be used to either align the "bytes" better to the current
/// hardware or perhaps to implement some cool behavior later on.
template <class Cell, class Cartridge> class Memory {
	/// Mapping of the non cartrigde memory:
	/// Source: 'https://wiki.nesdev.com/w/index.php/CPU_memory_map'
	///
	/// Note that 'size' refers to the non mirrored partion of the memory.
	///
	/// Internal RAM:
	///     range: (0x0000, 0x1FFF)
	///     size: 0x0800 (2048 B)
	///     mirror: (0x0000, 0x07FF)
	///     offset: 0x0000
	///
	/// PPU registers:
	///     range: (0x2000, 0x3FFF)
	///     size: 0x0008 (8 B)
	///     mirror: (0x2000, 0x2007)
	///     offset: 0x0800
	///
	/// NES APU and I/O registers:
	///     range: (0x4000, 0x401F)
	///     size: 0x0020 (36 B)
	///     mirror: none
	///     offset: 0x0808
	///
	/// Cartrigde space
	///     range: (0x4020, 0xFFFF)
	///

	std::array<Cell, 0x4020> nesMemory;

	std::size_t AdjustAddr(std::size_t addr)
	{
		if (addr < 0x0800)
			return 0x0000 + (addr & 0x07FF);
		if (addr < 0x4000)
			return 0x0800 + (addr & 0x0007);
		// NOTE:
		// It is mentioned at that addresses 0x4018 to 0x401F are
		// normally disabled.
		if (addr < 0x4020)
			return 0x0808 + (addr & 0x001F);

		// TODO:
		// Not sure how to do cartridge memory.
		return 0;
	}

    public:
	/// Reads the memory at addr.
	std::uint8_t Read(std::uint16_t addr)
	{
		return nesMemory[AdjustAddr(addr)];
	}

	/// Write data to the memory at addr.
	void Write(std::uint8_t data, std::uint16_t addr)
	{
		nesMemory[AdjustAddr(addr)] = data;
	}
};

} // namespace nemu
