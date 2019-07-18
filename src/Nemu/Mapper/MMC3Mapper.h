// ---------------------------------------------------------------------* C++ *-
// MMC3Mapper.h
//
// -----------------------------------------------------------------------------

#pragma once

#include <vector>
#include <cstddef>
#include <memory>
#include "Nemu/Core/CPU.h"
#include "Nemu/Core/PPU.h"
#include "Nemu/Mapper/CPUMapper.h"

namespace nemu::mapper {

    /// Provides mapping for the MMC3 cartridge layout.
    ///
    /// Mapping for PRG:  
    ///          (Fixed, PRG-RAM):
    ///           range: (0x6000, 0x7FFF)
    ///           size:  0x2000 (8kB)
    ///
    ///    Slot0 OR Slot2 (Switchable, PRG-ROM):
    ///           range: (0x8000, 0x9FFF)
    ///           size:  0x2000 (8kB)
    ///
    ///    Slot1 (Switchable, PRG-ROM):
    ///           range: (0xA000, 0xBFFF)
    ///           size:  0x2000 (8kB)
    ///
    ///    Slot2 OR Slot0 (Switchable, PRG-ROM):
    ///           range: (0xC000, 0xDFFF)
    ///           size:  0x2000 (8kB)
    ///
    ///    Slot3 (Switchable, PRG-ROM):
    ///           range: (0xE000, 0xFFFF)
    ///           size:  0x2000 (8kB)
    ///
    /// Mapping for CHR:
    ///    Slot0 (Switchable, CHR-ROM/RAM):
    ///           range: (0x0000, 0x3FF)
    ///           size:  0x400 (1kB)
    ///    Slot1 (Switchable, CHR-ROM/RAM):
    ///           range: (0x400, 0x7FF)
    ///           size:  0x400 (1kB)
    ///    Slot2 (Switchable, CHR-ROM/RAM):
    ///           range: (0x800, 0xBFF)
    ///           size:  0x400 (1kB)
    ///    Slot3 (Switchable, CHR-ROM/RAM):
    ///           range: (0xC00, 0x0FFF)
    ///           size:  0x400 (1kB)
    ///    Slot4 (Switchable, CHR-ROM/RAM):
    ///           range: (0x1000, 0x13FF)
    ///           size:  0x400 (1kB)
    ///    Slot5 (Switchable, CHR-ROM/RAM):
    ///           range: (0x1400, 0x17FF)
    ///           size:  0x400 (1kB)
    ///    Slot6 (Switchable, CHR-ROM/RAM):
    ///           range: (0x1800, 0x1BFF)
    ///           size:  0x400 (1kB)
    ///    Slot7 (Switchable, CHR-ROM/RAM):
    ///           range: (0x1C00, 0x1FFF)
    ///           size:  0x400 (1kB)

    class MMC3Mapper {
        std::vector<unsigned> prgROM, prgRAM, chrRAM;
		std::array<std::uint8_t, 8> regs;
        std::uint8_t regControl;

		std::array<std::size_t, 4> prgSlot;
		std::array<std::size_t, 8> chrSlot;

        unsigned irqPeriod, irqCounter;
        bool irqEnabled;

    public:
        std::shared_ptr<PPU<PPUMapper<MMC3Mapper>>> ppu;
        std::shared_ptr<CPU<CPUMapper<MMC3Mapper>>> cpu;

        MMC3Mapper(std::vector<unsigned>&& prg, std::vector<unsigned>&& chr)
            : prgROM(std::move(prg))
            , prgRAM(0x2000)
            , chrRAM(std::move(chr))
            , regs{}
            , regControl()
            , prgSlot{}
            , chrSlot{}
            , irqPeriod(0)
            , irqCounter(0)
            , irqEnabled(false)
        {
            if (chrRAM.size() == 0)
				chrRAM = std::vector<unsigned>(0x2000);
        }


		void SetPRGROM(std::vector<unsigned>&& newData)
		{
			prgROM = std::move(newData);
		}

		void SetCHRROM(std::vector<unsigned>&& newData)
		{
			chrRAM = std::move(newData);
		}

        void Update()
        {
            prgSlot[1] = 0x2000 * regs[7]; // Switchable, but always in same address range 
            prgSlot[3] = prgROM.size() - 0x2000; // Fixed to last bank

            // PRG Mode 0:
            if (!(regControl & 0b01000000)) {
				prgSlot[0] = 0x2000 * regs[6];
				prgSlot[2] = prgROM.size() - 0x2000 * 2; // Second to last bank
            }
            // PRG Mode 1:
            else {
				prgSlot[0] = prgROM.size() - 0x2000 * 2; // Second to last bank
				prgSlot[2] = 0x2000 * regs[6];
            }

            // CHR Mode 0:
            if (!(regControl & 0b10000000)) {
                // 2kB Blocks
				chrSlot[0] = 0x800 * (regs[0] >> 1);
				chrSlot[1] = chrSlot[0] + 0x400;
				chrSlot[2] = 0x800 * (regs[1] >> 1);
				chrSlot[3] = chrSlot[2] + 0x400;

                // 1kB Blocks
				chrSlot[4] = 0x400 * regs[2];
                chrSlot[5] = 0x400 * regs[3];
				chrSlot[6] = 0x400 * regs[4];
				chrSlot[7] = 0x400 * regs[5];
            }
            // CHR Mode 1:
            else {
                // 1kB Blocks
				chrSlot[0] = 0x400 * regs[2];
				chrSlot[1] = 0x400 * regs[3];
				chrSlot[2] = 0x400 * regs[4];
				chrSlot[3] = 0x400 * regs[5];

                // 2kB Blocks
				chrSlot[2] = 0x800 * (regs[0] >> 1);
				chrSlot[3] = 0x800 * (regs[1] >> 1);
            }

        }

        std::uint8_t ReadPRG(std::size_t address)
        {
            if (address < 0x6000)
                return 0;
            if (address <= 0x7FFF)
                return prgRAM[address % 0x2000];
            if (address <= 0x9FFF)
                return prgROM[prgSlot[0] + address % 0x2000];
			if (address <= 0xBFFF)
				return prgROM[prgSlot[1] + address % 0x2000];
			if (address <= 0xDFFF)
				return prgROM[prgSlot[2] + address % 0x2000];
			return prgROM[prgSlot[3] + address % 0x2000];
        }

        void WritePRG(std::size_t address, std::uint8_t value)
        {
            // [0x6000, 0x7FFF] Internal Ram
            if (address < 0x8000)
                prgRAM[address % 0x2000] = value;
            // [0x8000, 0xFFFF] Bankswitching
            else if (address & 0x8000) {
                switch (address & 0xE001) {
                case 0x8000: regControl = value;               break;
                case 0x8001: regs[regControl & 0b111] = value; break;
                case 0xA000: SetMirroring(value & 1);          break;
                case 0xC000: irqPeriod = value;                break;
                case 0xC001: irqCounter = 0;                   break;
                case 0xE000: {
                    cpu->SetIRQ(false);
                    irqEnabled = false;
                    break;
                }
                case 0xE001: irqEnabled = true;                break;
                }
                Update();
            }
        }

        std::uint8_t ReadCHR(std::size_t address)
        {
			if (address <= 0x03FF)
				return chrRAM[chrSlot[0] + address % 0x400];
            if (address <= 0x07FF)
                return chrRAM[chrSlot[1] + address % 0x400];
            if (address <= 0x0BFF)
                return chrRAM[chrSlot[2] + address % 0x400];
            if (address <= 0x0FFF)
                return chrRAM[chrSlot[3] + address % 0x400];
            if (address <= 0x13FF)
                return chrRAM[chrSlot[4] + address % 0x400];
            if (address <= 0x17FF)
                return chrRAM[chrSlot[5] + address % 0x400];
            if (address <= 0x1BFF)
                return chrRAM[chrSlot[6] + address % 0x400];
            if (address <= 0x1FFF)
                return chrRAM[chrSlot[7] + address % 0x400];
            return 0;
        }

        void WriteCHR(std::size_t address, std::uint8_t value)
        {
            if (address <= 0x03FF)
                chrRAM[chrSlot[0] + address % 0x400] = value;
            if (address <= 0x07FF)
				chrRAM[chrSlot[1] + address % 0x400] = value;
            if (address <= 0x0BFF)
				chrRAM[chrSlot[2] + address % 0x400] = value;
            if (address <= 0x0FFF)
				chrRAM[chrSlot[3] + address % 0x400] = value;
            if (address <= 0x13FF)
				chrRAM[chrSlot[4] + address % 0x400] = value;
            if (address <= 0x17FF)
				chrRAM[chrSlot[5] + address % 0x400] = value;
            if (address <= 0x1BFF)
				chrRAM[chrSlot[6] + address % 0x400] = value;
            if (address <= 0x1FFF)
				chrRAM[chrSlot[7] + address % 0x400] = value;
        }

        void OnScanline()
        {
            if (irqCounter == 0)
                irqCounter = irqPeriod;
            else
                irqCounter--;

            if (irqEnabled && irqCounter == 0)
                cpu->SetIRQ(true);
        }

        void SetMirroring(bool horizontal)
        {
            if (horizontal)
                ppu->SetMirroring(ppu::MirroringMode::Horizontal);
            else
                ppu->SetMirroring(ppu::MirroringMode::Vertical);
        }
    };

} // namespace mapper