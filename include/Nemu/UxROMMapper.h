// ---------------------------------------------------------------------* C++ *-
// UNROMMapper.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/NESMapper.h"
#include <vector>
#include <cstddef>

namespace nemu {

	/// Provides mapping for the UNROM / UOROM cartridge layout.
	///
	/// Mapping:
	///    Block0 (Switchable):
	///           range: (0x8000, 0xBFFF)
	///           size: 0x4000 * N (16kB * N)
	///           modulus: 0x4000
	///
	///    Block1 (Fixed):
	///           range: (0xC000, 0xFFFF)
	///           size: 0x4000 (16kB)
	///           modulus: 0x4000
	///

	class UxROMMapper : public NESMapper {
		using Iterator = std::vector<unsigned>::iterator;
		std::vector<unsigned> data;
		Iterator lastBank;
		Iterator currentBank;

	public:
		UxROMMapper(const Iterator& begin, const Iterator& end)
			: data(begin, end)
			, currentBank(data.begin())
			, lastBank(std::prev(data.end(), 0x4000))
		{}

		virtual std::uint8_t Read(std::size_t address) override
		{
			if (address < 0x8000)
				return 0;
			if(address <= 0xBFFF)
				return static_cast<std::uint8_t>(*std::next(currentBank, address % 0x4000));
			return static_cast<std::uint8_t>(*std::next(lastBank, address % 0x4000));
		}

		// Bankswitching
		virtual void Write(std::size_t address, std::uint8_t value) override
		{
			// If address is in range [0x8000, 0xFFFF]
			if (address & 0x8000)
				currentBank = std::next(data.begin(), 0x4000 * (value & 0xF));
		}
	};

} // namespace nemu
