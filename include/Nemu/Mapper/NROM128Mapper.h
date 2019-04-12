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
/// Mapping for PRG:
///     Slot 0:
///         range: (0x8000, 0xBFFF)
///         size: 0x4000 (16kB)
///     Slot 1:
///         range: (0xC000, 0xFFFF)
///         size: 0x4000 (16kB)
///
/// Mapping for CHR:
///     Slot 0:
///         range: (0x0000, 0x0FFF)
///         size: 0x2000 (8kB)
///
class NROM128Mapper {
    std::vector<unsigned> prgROM, prgRAM;
    std::vector<unsigned> chrRAM;

public:
       std::shared_ptr<PPU<PPUMapper<NROM128Mapper>>> ppu;
       std::shared_ptr<CPU<CPUMapper<NROM128Mapper>>> cpu;

    NROM128Mapper(std::vector<unsigned>&& prgROM, std::vector<unsigned>&& chrROM) 
        : prgROM(std::move(prgROM))
        , prgRAM(0x2000)
        , chrRAM(std::move(chrROM))
    {}

    void SetPRGROM(std::vector<unsigned>&& newData)
    { prgROM = std::move(newData); }

    void SetCHRROM(std::vector<unsigned>&& newData)
    { chrRAM = std::move(newData); }

    std::uint8_t ReadPRG(std::size_t address)
    {
        if (address < 0x6000)
            return 0;
        if (address <= 0x7FFF)
            return prgRAM[address % 0x2000];
        return prgROM[address % 0x4000];
    }

    void WritePRG(std::size_t address, std::uint8_t value) {}

    std::uint8_t ReadCHR(std::size_t address)
    {
        return chrRAM[address];
    }

    void WriteCHR(std::size_t address, std::uint8_t value) {}
    void OnScanline() {}
};

} // namespace mapper
} // namespace nemu
