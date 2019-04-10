// -----------------------------------------------------------------------------------------* C++ *-
// UNROMMapper.h
//
// -------------------------------------------------------------------------------------------------

#pragma once

#include <vector>
#include <cstddef>

namespace nemu {
namespace mapper {

/// Provides mapping for the UNROM / UOROM cartridge layout.
///
/// Mapping:
///    Block0 (Switchable):
///        range: (0x8000, 0xBFFF)
///        size: 0x4000 * N (16kB * N)
///        modulus: 0x4000
///    Block1 (Fixed):
///        range: (0xC000, 0xFFFF)
///        size: 0x4000 (16kB)
///        modulus: 0x4000
///
class UxROMMapper {
    std::vector<unsigned> data;

    std::vector<unsigned>::iterator lastBank;
    std::vector<unsigned>::iterator currentBank;

public:
    UxROMMapper(std::vector<unsigned>::const_iterator begin, 
                std::vector<unsigned>::const_iterator end) 
        : data(begin, end)
        , currentBank(data.begin())
        , lastBank(std::prev(data.end(), 0x4000))
    {}

    std::uint8_t Read(std::size_t address)
    {
        if (address < 0x8000)
            return 0;
        if(address <= 0xBFFF)
            return static_cast<std::uint8_t>(*std::next(currentBank, address % 0x4000));
        return static_cast<std::uint8_t>(*std::next(lastBank, address % 0x4000));
    }

    void Write(std::size_t address, std::uint8_t value)
    {
        // Bankswitching if address is in range [0x8000, 0xFFFF]
        if (address & 0x8000)
            currentBank = std::next(data.begin(), 0x4000 * (value & 0xF));
    }
};

} // namespace mapper
} // namespace nemu
