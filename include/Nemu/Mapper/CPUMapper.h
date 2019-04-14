// -----------------------------------------------------------------------------------------* C++ *-
// CPUMapper.h
//
// -------------------------------------------------------------------------------------------------

#pragma once

#include "Nemu/CPU.h"
#include "Nemu/Joypad.h"
#include "Nemu/PPU.h"
#include "Nemu/Mapper/PPUMapper.h"
#include <memory>

namespace nemu {
namespace mapper {

/// Mapper for the memory accesable by the CPU. It consists of two parts; a internal mapper which is
/// always the same and a mapper that is dependent on the mapper of the cartridge.
///
/// Mapping:
///     Internal RAM:
///         range: (0x0000, 0x1FFF)
///         size: 0x0800 (2048 B)
///         mask: 0x07FF
///         offset: 0x0000
///     PPU registers:
///         range: (0x2000, 0x3FFF)
///         size: 0x0008 (8 B)
///         mask: 0x0007
///     NES APU and I/O registers:
///         range: (0x4000, 0x401F)
///         size: 0x0020 (36 B)
///    CartridgeMapper:
///        range: (0x4020, 0xFFFF)
///        size: depends on mapper class.
///        mirroring: depends on mapper class.
///
/// Source: 'https://wiki.nesdev.com/w/index.php/CPU_memory_map'
///
template <class CartridgeMapper>
class CPUMapper {
    static constexpr unsigned InternalRAMAddressMask = 0x07FF;
    static constexpr unsigned PPUAddressMask = 0x0007;

  public:
    // TODO:
    //  For simplicity everything is shared pointers. There are probably more static solutions.
    std::shared_ptr<CPU<CPUMapper<CartridgeMapper>>> cpu;
    std::shared_ptr<PPU<PPUMapper<CartridgeMapper>>> ppu;
    std::shared_ptr<CartridgeMapper> cartridgeMapper;

    Joypad joypad;

    std::vector<unsigned> internalRAM;

    CPUMapper()
        : internalRAM(0x0800)
    {}

    std::uint8_t Read(std::size_t address)
    {
        if (address <= 0x1FFF)
            return internalRAM[address & InternalRAMAddressMask];
        if (address <= 0x3FFF) {
            switch (address & PPUAddressMask) {
            case 2: return ppu->ReadPPUSTATUS();
            case 4: return ppu->ReadOAMDATA();
            case 7: return ppu->ReadPPUDATA();
            default: break;
            }
            return 0;
        }
        if (address == 0x4016) {
            return joypad.Read(0);
        }
        if (address == 0x4017) {
            return joypad.Read(1);
        }
        if (address <= 0x401F) {
            // TODO: Implement APU.
            return 0;
        }
        if (address <= 0xFFFF)
            return cartridgeMapper->ReadPRG(address);
        return 0;
    }

    void Write(std::size_t address, std::uint8_t value)
    {
        if (address <= 0x1FFF) {
            internalRAM[address & InternalRAMAddressMask] = value;
        }
        else if (address <= 0x3FFF) {
            switch (address & PPUAddressMask) {
            case 0: ppu->WritePPUCTRL(value);   break;
            case 1: ppu->WritePPUMASK(value);   break;
            case 3: ppu->WriteOAMADDR(value);   break;
            case 4: ppu->WriteOAMDATA(value);   break;
            case 5: ppu->WritePPUSCROLL(value); break;
            case 6: ppu->WritePPUADDR(value);   break;
            case 7: ppu->WritePPUDATA(value);   break;
            default: break;
            }
        }        
        else if (address == 0x4014) {
            cpu->DmaOam(value);
        }
        else if (address == 0x4016) {
            return joypad.Write(value & 1);
        }
        else if (address <= 0x401F) {
            // TODO: Implement the entire APU.
        }
        else if (address <= 0xFFFF) {
            cartridgeMapper->WritePRG(address, value);
        }
    }
};

} // namespace mapper
} // namespace nemu