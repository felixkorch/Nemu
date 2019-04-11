// -----------------------------------------------------------------------------------------* C++ *-
// NROM128Mapper.h
//
// -------------------------------------------------------------------------------------------------

#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <type_traits>
#include <vector>
#include <algorithm>

#include <iostream>

namespace nemu {
namespace mapper {

/// Provides mapping for the NROM-128 cartridge layout.
///
/// PRG-ROM:
///     Slot 0:
///         range: (0x8000, 0xBFFF)
///         size: 0x4000 (16kB)
///         mirroring: None
///     Slot 1:
///         range: (0xC000, 0xFFFF)
///         size: 0x4000 (16kB)
///         mirroring: Mirrors Slot 0
///
/// CHR-ROM:
///     Slot 0:
///         range: (0x0000, 0x0FFF)
///         size: 0x1000 (8kB)
///         mirroring: None
///
class NROM128Mapper {
    std::vector<unsigned> prgROM;
    std::vector<unsigned> chrROM;

   public:
    NROM128Mapper() 
        : prgROM(0x4000)
        , chrROM(0x1000)
    {}

    /*
    template <
        class Iterator,
        typename std::enable_if<
            std::is_integral<typename std::iterator_traits<Iterator>::value_type>
                ::value, 
            int>::type = 0
    >
    */
    template <class Iterator>
    void LoadPRGROM(Iterator begin, Iterator end)
    { std::copy(begin, end, prgROM.begin()); }

    /*
    template <
        class Iterator,
        typename std::enable_if<
            std::is_integral<typename std::iterator_traits<Iterator>::value_type>
                ::value, 
            int>::type = 0    
    >
    */
    template <class Iterator>
    void LoadCHRROM(Iterator begin, Iterator end)
    { std::copy(begin, end, chrROM.begin()); }

    std::uint8_t ReadPRG(std::size_t address)
    {
        if (address <= 0x7FFF)
            return 0;
        return prgROM[address % 0x4000];
    }

    void WritePRG(std::size_t address, std::uint8_t value) {}

    std::uint8_t ReadCHR(std::size_t address)
    {
        if (address <= 0x0FFF)
            return chrROM[address];
        return 0;
    }

    void WriteCHR(std::size_t address, std::uint8_t value) {}
};

} // namespace mapper
} // namespace nemu
