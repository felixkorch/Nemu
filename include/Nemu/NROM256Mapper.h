// ---------------------------------------------------------------------* C++ *-
// NROM256Mapper.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/NESMapper.h"
#include <vector>
#include <cstddef>

namespace nemu {

/// Provides mapping layout for the NROM-256 cartridge.
///
/// Mapping:
///     Block0:
///             range: (0x8000, 0xFFFF)
///             size: 0x8000 (32kB)
///
class NROM256Mapper {
	using Iterator = std::vector<unsigned>::iterator;
    std::vector<unsigned> data;

   public:
    NROM256Mapper(const Iterator& begin, const Iterator& end)
		: data(begin, end)
	{}

    NROM256Mapper(std::vector<unsigned>::const_iterator begin, 
                  std::vector<unsigned>::const_iterator end) 
        : data(begin, end) 
    {}

    std::uint8_t Read(std::size_t address)
	{
        if (address < 0x8000)
            return 0;
        return static_cast<std::uint8_t>(data[address % 0x8000]);
    }

    void Write(std::size_t address, std::uint8_t value)
	{
        if (address < 0x8000)
            return;
        data[address % 0x8000] = value;
    }
};

} // namespace nemu