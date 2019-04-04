// ---------------------------------------------------------------------* C++ *-
// NROM128Mapper.h
//
// -----------------------------------------------------------------------------

#pragma once

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
class NROM128Mapper {
    std::array<unsigned, 0x4000> data;

   public:
    template <class Iterator>
    NROM128Mapper(const Iterator& begin, const Iterator& end) {
        auto it = begin;
        auto address = 0x8000;
        while (it != end)
            Write(address++, *it++);
    }

    std::uint8_t Read(std::size_t address) {
        if (address < 0x8000)
            return 0;
        return static_cast<std::uint8_t>(data[address % 0x4000]);
    }

    void Write(std::size_t address, std::uint8_t value) {
        if (address < 0x8000)
            return;
        data[address % 0x4000] = value;
    }
};

} // namespace nemu