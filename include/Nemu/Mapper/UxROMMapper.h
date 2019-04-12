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
/// Mapping for PRG:
///    Slot0 (Switchable):
///        range: [0x8000, 0xBFFF]
///        size: 0x4000
///    Slot1 (Fixed):
///        range: [0xC000, 0xFFFF]
///        size: 0x4000 (16kB)
///

class UxROMMapper {
    using Iterator = std::vector<unsigned>::iterator;

    std::vector<unsigned> prgROM, prgRAM, chrRAM;
    Iterator prgSlots[2];

public:
    std::shared_ptr<PPU<PPUMapper<UxROMMapper>>> ppu;
    std::shared_ptr<CPU<CPUMapper<UxROMMapper>>> cpu;

    UxROMMapper(std::vector<unsigned>&& prg, std::vector<unsigned>&& chr) 
        : prgROM(std::move(prg))
        , prgRAM(0x2000)
        , chrRAM(0x2000)
        , prgSlots{ prgROM.begin(), std::prev(prgROM.end(), 0x4000) }
    {}

    std::uint8_t ReadPRG(std::size_t address)
    {
        if (address < 0x8000)
            return 0;
        if (address <= 0xBFFF)
            return *std::next(prgSlots[0], address % 0x4000);
        return *std::next(prgSlots[1], address % 0x4000);
    }

    void WritePRG(std::size_t address, std::uint8_t value)
    {
        // Bankswitching if address is in range [0x8000, 0xFFFF]
        if (address & 0x8000)
            prgSlots[0] = std::next(prgROM.begin(), 0x4000 * (value & 0xF));
    }

    std::uint8_t ReadCHR(std::size_t address)
    {
        return chrRAM[address % 0x2000];
    }
    void WriteCHR(std::size_t address, std::uint8_t value)
    {
        chrRAM[address % 0x2000] = value;
    }
    void OnScanline() {}
};

} // namespace mapper
} // namespace nemu
