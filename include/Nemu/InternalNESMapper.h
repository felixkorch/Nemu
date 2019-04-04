// ---------------------------------------------------------------------* C++ *-
// InternalNESMapper.h
//
// -----------------------------------------------------------------------------

#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include "Nemu/Joypad.h"
#include "Nemu/PPU.h"

namespace nemu {

/// Mapping of the non cartrigde memory:
/// Source: 'https://wiki.nesdev.com/w/index.php/CPU_memory_map'
///
/// Internal RAM:
///     range: (0x0000, 0x1FFF)
///     size: 0x0800 (2048 B)
///     mask: 0x07FF
///     offset: 0x0000
///
/// PPU registers:
///     range: (0x2000, 0x3FFF)
///     size: 0x0008 (8 B)
///     mask: 0x0007
///
/// NES APU and I/O registers:
///     range: (0x4000, 0x401F)
///     size: 0x0020 (36 B)
///
class InternalNESMapper {
    static constexpr unsigned InternalRAMAddressMask = 0x07FF;
    static constexpr unsigned PPUAddressMask = 0x0007;

    std::array<unsigned, 0x0800> internalRAM;
    std::shared_ptr<PPU> ppu;
    // TODO: APU is not fully implemented
    Joypad joypad;

   public:
    InternalNESMapper(std::shared_ptr<PPU> ppu) : ppu(ppu) {}

    std::uint8_t Read(std::size_t address) {
        if (address < 0x2000)
            return static_cast<std::uint8_t>(
                internalRAM[address & InternalRAMAddressMask]);
        if (address < 0x4000) {
            switch (address & PPUAddressMask) {
                case 2:
                    return ppu->ReadPPUSTATUS();
                case 4:
                    return ppu->ReadOAMDATA();
                case 7:
                    return ppu->ReadPPUDATA();
                default:
                    break;
            }
            return 0;
        }
        if (address == 0x4016) {
            return joypad.Read(0);
        }
        // TODO: APU
        // if (address < 0x4020) {}
        return 0;
    }

    void Write(std::size_t address, std::uint8_t value) {
        if (address < 0x2000) {
            internalRAM[address & InternalRAMAddressMask] = value;
        } else if (address < 0x4000) {
            switch (address & PPUAddressMask) {
                case 0:
                    ppu->WritePPUCTRL(value);
                    break;
                case 1:
                    ppu->WritePPUMASK(value);
                    break;
                case 3:
                    ppu->WriteOAMADDR(value);
                    break;
                case 4:
                    ppu->WriteOAMDATA(value);
                    break;
                case 5:
                    ppu->WritePPUSCROLL(value);
                    break;
                case 6:
                    ppu->WritePPUADDR(value);
                    break;
                case 7:
                    ppu->WritePPUDATA(value);
                    break;
                default:
                    break;
            }
        } else if (address == 0x4016) {
            return joypad.Write(value & 1);
        }
        // TODO: Implement APU
        // else if (address < 0x4020) {
        // }
        else {
            return;
        }
    }
};

}  // namespace nemu
