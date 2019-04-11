// ---------------------------------------------------------------------* C++ *-
// MMC1Mapper.h
//
// -----------------------------------------------------------------------------

#pragma once

#include <vector>
#include <cstddef>

namespace nemu {
namespace mapper {

/// Provides mapping for the MMC1 / SxROM cartridge layout.
///
/// Mapping:
///    Block0 (Fixed, PRG-RAM):
///           range: (0x6000, 0x7FFF)
///           size: 0x2000 (8KB)
///           modulus: -
///
///    Block1 (Switchable, PRG-ROM):
///           range: (0x8000, 0xBFFF)
///           size: 0x4000 (16kB)
///           modulus: 0x4000
///
///    Block2 (Switchable, PRG-ROM):
///           range: (0xC000, 0xFFFF)
///           size: 0x4000 (16kB)
///           modulus: 0x4000
///
class MMC1Mapper {
    using Iterator = std::vector<unsigned>::iterator;
    std::vector<unsigned> rom;
    std::vector<unsigned> ram;
    std::uint8_t regs[4];      // Internal registers ([0]: Control, [1]: CHR-bank 0, [2]: CHR-bank 1, [3]: PRG-bank)
    Iterator block1, block2;
    bool oneBlock;             // One big 32KB block?

public:
    std::shared_ptr<PPU<MMC1Mapper>> ppu;

    MMC1Mapper(const Iterator& begin, const Iterator& end)
        : rom(begin, end)
        , ram(0x2000)
        , regs{ 0x0C, 0, 0, 0 }
        , block1()
        , block2()
        , oneBlock()
    {
        UpdateSlots();
    }

    MMC1Mapper(std::vector<unsigned>::const_iterator begin, std::vector<unsigned>::const_iterator end)
        : rom(begin, end)
        , ram(0x2000)
        , regs{ 0x0C, 0, 0, 0 }
        , block1()
        , block2()
        , oneBlock()
    {
        UpdateSlots();
    }

    void UpdateSlots()
    {
        // Determines whether the whole 32KB block is fixed
        // 16KB
        if (regs[0] & 0b1000) {
            // [0x8000, 0xBFFF] switchable, [0xC000, 0xFFFF] fixed
            if (regs[0] & 0b100) {
                block1 = std::next(rom.begin(), 0x4000 * (regs[3] & 0xF));
                block2 = std::prev(rom.end(), 0x4000); // Last bank
            }
            // [0x8000, 0xBFFF] fixed, [0xC000, 0xFFFF] switchable
            else {
                block1 = rom.begin();
                block2 = std::next(rom.begin(), 0x4000 * (regs[3] & 0xF));
            }
            oneBlock = false;
        }
        // 32KB
        else {
            block1 = std::next(rom.begin(), 0x4000 * ((regs[3] & 0xF) >> 1));
            oneBlock = true;
        }

        // TODO: Set CHR-banks, Set mirroring

    }

    std::uint8_t ReadPRG(std::size_t address) { return 0; }
    void WritePRG(std::size_t address, std::uint8_t value) {}

    std::uint8_t ReadCHR(std::size_t address) { return 0; }
    void WriteCHR(std::size_t address, std::uint8_t value) {}

    std::uint8_t Read(std::size_t address)
    {
        if (address < 0x6000)
            return 0;

        if (address <= 0x7FFF)
            return static_cast<std::uint8_t>(ram[address % 0x2000]);

        if (oneBlock)
            return static_cast<std::uint8_t>(*std::next(block1, address % 0x8000));

        if (address <= 0xBFFF)
            return static_cast<std::uint8_t>(*std::next(block1, address % 0x4000));

        return static_cast<std::uint8_t>(*std::next(block2, address % 0x4000));
    }

    void Write(std::size_t address, std::uint8_t value)
    {
        static unsigned write = 0;
        static std::uint8_t shiftReg = 0; // Holds the bank number after 5 writes

        // [0x6000, 0x7FFF] Internal Ram
        if (address <= 0x7FFF) {
            ram[address % 0x2000] = value;
        }
        // [0x8000, 0xFFFF] Bankswitching
        else if (address <= 0xFFFF) {
            // Reset everything to the default values
            if (value & 0x80) {
                write = 0;
                shiftReg = 0;
                regs[0] |= 0x0C;
                UpdateSlots();
            }
            else {
                shiftReg = ((value & 1) << 4) | (shiftReg >> 1);
                if (++write == 5) {
                    // On the 5th write, fill the apprioprate internal register with the data of the shift register
                    // Bit 13 & 14 tells which register to write to
                    regs[(address >> 13) & 0b11] = shiftReg;
                    write = 0;
                    shiftReg = 0;
                    UpdateSlots();
                }
            }
        }
    }
};

} // namespace mapper
} // namespace nemu
