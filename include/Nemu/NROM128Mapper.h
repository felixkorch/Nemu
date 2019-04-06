// ---------------------------------------------------------------------* C++ *-
// NROM128Mapper.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/NESMapper.h"
#include <array>
#include <cstddef>

namespace nemu {

/// Provides mapping for the NROM-128 cartridge layout.
///
/// Mapping:
///     Block0:
///             range: (0x8000, 0xFFFF)
///             size: 0x8000 (16kB)
///             modulus: 0x4000
///
class NROM128Mapper : public NESMapper {
	using Iterator = std::vector<unsigned>::iterator;
    std::vector<unsigned> data;

   public:
    NROM128Mapper(const Iterator& begin, const Iterator& end)
		: data(begin, end)
	{}

    virtual std::uint8_t Read(std::size_t address) override
	{
        if (address < 0x8000)
            return 0;
        return static_cast<std::uint8_t>(data[address % 0x4000]);
    }

    virtual void Write(std::size_t address, std::uint8_t value) override
	{
        if (address < 0x8000)
            return;
        data[address % 0x4000] = value;
    }
};

} // namespace nemu