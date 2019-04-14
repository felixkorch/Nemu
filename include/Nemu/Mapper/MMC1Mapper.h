// ---------------------------------------------------------------------* C++ *-
// MMC1Mapper.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/PPU.h"
#include "Nemu/Mapper/PPUMapper.h"
#include "Nemu/Mapper/CPUMapper.h"
#include <cstddef>
#include <memory>
#include <vector>

namespace nemu {
namespace mapper {

    /// Provides mapping for the MMC1 / SxROM cartridge layout.
    ///
    /// Mapping for PRG:
    ///           (Fixed, PRG-RAM):
    ///           range: (0x6000, 0x7FFF)
    ///           size: 0x2000 (8KB)
    ///
    ///    Slot0 (Switchable, PRG-ROM):
    ///           range: (0x8000, 0xBFFF)
    ///           size: 0x4000 (16kB)
    ///
    ///    Slot1 (Switchable, PRG-ROM):
    ///           range: (0xC000, 0xFFFF)
    ///           size: 0x4000 (16kB)
    ///
    /// Mapping for CHR:
    ///    Slot0 (Switchable, CHR-ROM/RAM):
    ///           range: (0x0000, 0x0FFF)
    ///           size: 0x1000 (4kB)
    ///
    ///    Slot1 (Switchable, CHR-ROM/RAM):
    ///           range: (0x1000, 0x1FFF)
    ///           size: 0x1000 (4kB)

    class MMC1Mapper {
        using Iterator = std::vector<unsigned>::iterator;
        std::vector<unsigned> prgROM, prgRAM, chrRAM;
        std::uint8_t regs[4]; // Internal registers - [0]: Control, [1]: CHR-bank 0, [2]: CHR-bank 1, [3]: PRG-bank
        Iterator prgSlot[2];
        Iterator chrSlot[2];

    public:
        std::shared_ptr<PPU<PPUMapper<MMC1Mapper>>> ppu;
        std::shared_ptr<CPU<CPUMapper<MMC1Mapper>>> cpu;

        MMC1Mapper(std::vector<unsigned>&& prg, std::vector<unsigned>&& chr)
            : prgROM(std::move(prg))
            , prgRAM(0x2000)
            , chrRAM(std::move(chr))
            , regs{ 0x0C, 0, 0, 0 }
            , prgSlot{}
            , chrSlot{}
        {
            if (chrRAM.size() == 0) chrRAM = std::vector<unsigned>(0x2000);
        }

        void Update()
        {
            // Determines whether the whole 32KB block is fixed
            // 16KB
            if (regs[0] & 0b1000) {
                // [0x8000, 0xBFFF] switchable, [0xC000, 0xFFFF] fixed
                if (regs[0] & 0b100) {
                    prgSlot[0] = std::next(prgROM.begin(), 0x4000 * (regs[3] & 0xF));
                    prgSlot[1] = std::prev(prgROM.end(), 0x4000); // Last bank
                }
                // [0x8000, 0xBFFF] fixed, [0xC000, 0xFFFF] switchable
                else {
                    prgSlot[0] = prgROM.begin();
                    prgSlot[1] = std::next(prgROM.begin(), 0x4000 * (regs[3] & 0xF));
                }
            }
            // 32KB
            else {
                prgSlot[0] = std::next(prgROM.begin(), 0x4000 * ((regs[3] & 0xF) >> 1));
                prgSlot[1] = std::next(prgSlot[0], 0x4000);
            }

            // 4KB CHR:
            if (regs[0] & 0b10000) {
                chrSlot[0] = std::next(chrRAM.begin(), 0x1000 * regs[1]);
                chrSlot[1] = std::next(chrRAM.begin(), 0x1000 * regs[2]);
            }
            // 8KB CHR:
            else {
                chrSlot[0] = std::next(chrRAM.begin(), 0x1000 * (regs[1] >> 1));
                chrSlot[1] = std::next(chrSlot[0], 0x1000);
            }
            // Set mirroring:
            switch (regs[0] & 0b11) {
            case 2: ppu->SetMirroring(ppu::MirroringMode::Vertical);   break;
            case 3: ppu->SetMirroring(ppu::MirroringMode::Horizontal); break;
            }

        }

        std::uint8_t ReadCHR(std::size_t address)
        {
            if (address <= 0x0FFF)
                return static_cast<std::uint8_t>(*std::next(chrSlot[0], address % 0x1000));
            if (address <= 0x1FFF)
                return static_cast<std::uint8_t>(*std::next(chrSlot[1], address % 0x1000));
            return 0;
        }

        void WriteCHR(std::size_t address, std::uint8_t value)
        {
            if (address <= 0x0FFF)
                *(chrSlot[0] + address % 0x1000) = value;
            else if(address <= 0x1FFF)
                *(chrSlot[1] + address % 0x1000) = value;
        }

        std::uint8_t ReadPRG(std::size_t address)
        {
            if (address < 0x6000)
                return 0;
            if (address <= 0x7FFF)
                return prgRAM[address % 0x2000];
            if (address <= 0xBFFF)
                return *std::next(prgSlot[0], address % 0x4000);
            return *std::next(prgSlot[1], address % 0x4000);
        }

        void WritePRG(std::size_t address, std::uint8_t value)
        {
            static unsigned write = 0;
            static std::uint8_t shiftReg = 0; // Holds the bank number after 5 writes

            // [0x6000, 0x7FFF] Internal Ram
            if (address <= 0x7FFF) {
                prgRAM[address % 0x2000] = value;
            }
            // [0x8000, 0xFFFF] Bankswitching
            else if (address <= 0xFFFF) {
                // Reset everything to the default values
                if (value & 0x80) {
                    write = 0;
                    shiftReg = 0;
                    regs[0] |= 0x0C;
                    Update();
                }
                else {
                    shiftReg = ((value & 1) << 4) | (shiftReg >> 1);
                    if (++write == 5) {
                        // On the 5th write, fill the apprioprate internal register with the data of the shift register
                        // Bit 13 & 14 tells which register to write to
                        regs[(address >> 13) & 0b11] = shiftReg;
                        write = 0;
                        shiftReg = 0;
                        Update();
                    }
                }
            }
        }

        void OnScanline() {}
    };

} // namespace mapper
} // namespace nemu
