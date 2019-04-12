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
    std::vector<unsigned> prgROM, prgRAM, chrRAM;

    std::vector<unsigned>::iterator lastBank;
    std::vector<unsigned>::iterator currentBank;

public:
    std::shared_ptr<PPU<PPUMapper<UxROMMapper>>> ppu;
    std::shared_ptr<CPU<CPUMapper<UxROMMapper>>> cpu;

    UxROMMapper(std::vector<unsigned>&& prg, std::vector<unsigned>&& chr) 
        : prgROM(std::move(prg))
        , prgRAM(0x2000)
        , chrRAM(0x2000)
        , currentBank(prgROM.begin())
        , lastBank(std::prev(prgROM.end(), 0x4000))
    {}

    std::uint8_t ReadPRG(std::size_t address)
    {
        if (address < 0x8000)
            return 0;
        if(address <= 0xBFFF)
            return static_cast<std::uint8_t>(*std::next(currentBank, address % 0x4000));
        return static_cast<std::uint8_t>(*std::next(lastBank, address % 0x4000));
    }

    void WritePRG(std::size_t address, std::uint8_t value)
    {
        // Bankswitching if address is in range [0x8000, 0xFFFF]
        if (address & 0x8000)
            currentBank = std::next(prgROM.begin(), 0x4000 * (value & 0xF));
    }

    std::uint8_t ReadCHR(std::size_t address)
    {
        if (address <= 0x1FFF)
            return chrRAM[address % chrRAM.size()];
        return 0;
    }
    void WriteCHR(std::size_t address, std::uint8_t value)
    {
        if (address <= 0x1FFF)
            chrRAM[address % chrRAM.size()] = value;
    }
    void OnNewScanline() {}
};

} // namespace mapper
} // namespace nemu
