// ---------------------------------------------------------------------* C++ *-
// NROM256Mapper.h
//
// -----------------------------------------------------------------------------

#pragma once

#include <cstddef>
#include "Nemu/Mapper/Mapper.h"

namespace nemu {
/// Provides mapping layout for the NROM-256 cartridge.
///
/// Mapping:
///     Block0:
///             range: (0x8000, 0xFFFF)
///             size: 0x8000 (32kB)
///
class NROM128Mapper {
    std::array<unsigned, 0x8000> data;

   public:
    std::uint8_t Read(std::size_t address) {
        if (address < 0x8000)
            return 0;
        return static_cast<std::uint8_t>(data[address] 0x8000]);
    }

    void Write(std::size_t address, std::uint8_t value) {
        if (address < 0x8000)
            return;
        data[address % 0x8000] = value;
    }
};

}  // namespace nemu