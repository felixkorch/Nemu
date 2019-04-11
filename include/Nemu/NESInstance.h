// -----------------------------------------------------------------------------------------* C++ *-
// NESInstance.h
//
// -------------------------------------------------------------------------------------------------

#pragma once

#include "Nemu/CPU.h"
#include "Nemu/Mapper/CPUMapper.h"
#include "Nemu/Mapper/NROM128Mapper.h"
#include "Nemu/Mapper/NROM256Mapper.h"
#include "Nemu/Mapper/MMC1Mapper.h"
#include "Nemu/Mapper/UxROMMapper.h"
#include "Nemu/NESInput.h"
#include "Nemu/PPU.h"
#include "Nemu/ROMLayout.h"
#include <cstdint>
#include <iostream>
#include <vector>

namespace nemu {

class NESInstance {
  public:
    virtual ~NESInstance() = default;
    virtual void RunFrame() = 0;
    virtual void Power() = 0;
    virtual std::vector<unsigned> DumpCPUMemory() = 0;
};

template <class CPUCartridgeMapper>
class NESInstanceBase: public NESInstance {
    // TODO:
    //  For simplicity everything is shared pointers. There are probably more static solutions.
    std::shared_ptr<mapper::CPUMapper<CPUCartridgeMapper>> cpuMapper;
    std::shared_ptr<CPU<mapper::CPUMapper<CPUCartridgeMapper>>> cpu;
    std::shared_ptr<PPU> ppu;

   public:
    NESInstanceBase(std::shared_ptr<PPU> ppu, std::shared_ptr<CPUCartridgeMapper> prgROM)
        : ppu(ppu)
    {
        // Create the memory of the CPU.
        cpuMapper = std::make_shared<typename decltype(cpuMapper)::element_type>();
        cpuMapper->cartridgeMapper = prgROM;

        // Create the CPU.
        cpu = std::make_shared<typename decltype(cpu)::element_type>(cpuMapper);

        // Connect CPU and PPU to the internal CPU memory.
        cpuMapper->cpu = cpu;
        cpuMapper->ppu = ppu;

        // Set connector lambdas for communication between CPU and PPU.
        ppu->SetNMI = [this]() { this->cpu->SetNMI(); };

        cpu->Tick = [this]() {
            this->ppu->Step();
            this->ppu->Step();
            this->ppu->Step();
            this->cpu->DecrementCycles();
        };
    }

    virtual void RunFrame() override
    { cpu->RunFrame(); }

    virtual void Power() override
    {
        cpu->Reset();
        ppu->Reset();
    }

    /// Placeholder idea for future implementation of save state and debugging tools.
    virtual std::vector<unsigned> DumpCPUMemory() override
    {
        std::vector<unsigned> data(0xFFFF);
        for(std::size_t i = 0; i < 0xFFFF; ++i)
            data[i] = cpuMapper->Read(i);
        return data;
    }

    void SetInput(const NESInput& input)
    { cpuMapper->joypad.SetInput(input); }
};

template <class CartridgePRGROM>
std::unique_ptr<NESInstanceBase<CartridgePRGROM>>
MakeNESInstance(const NESInput& input, std::shared_ptr<PPU> ppu,
                const std::vector<unsigned>& prgROM)
{
    std::unique_ptr<NESInstanceBase<CartridgePRGROM>> instance(new NESInstanceBase<CartridgePRGROM>(
        ppu, std::make_shared<CartridgePRGROM>(prgROM.cbegin(), prgROM.cend())));
    instance->SetInput(input);
    return instance;
}

std::unique_ptr<NESInstance>
MakeNESInstance(const std::string& path,
                NESInput& input,
                std::function<void(std::uint8_t*)> newFrameCallback)
{
    ROMLayout rom(path);

    auto prgROMSize = (rom.EndPRGROM() - rom.BeginPRGROM());
    auto chrROMSize = (rom.EndCHRROM() - rom.BeginCHRROM());

    std::cout << "PRG-ROM Size: " << prgROMSize << " B" << std::endl;
    std::cout << "PRG-RAM Size: " << (rom.EndPRGRAM() - rom.BeginPRGRAM()) << " B" << std::endl;
    std::cout << "CHR-ROM Size: " << chrROMSize << " B" << std::endl;

    std::vector<unsigned> prgROM(rom.BeginPRGROM(), rom.EndPRGROM());
    std::vector<unsigned> chrROM(rom.BeginCHRROM(), rom.EndCHRROM());
    std::vector<unsigned> chrRAM(0x2000);

    std::shared_ptr<PPU> ppu;

    if(chrROMSize == 0)
        ppu = std::make_shared<PPU>(std::move(chrRAM), newFrameCallback);
    else
        ppu = std::make_shared<PPU>(std::move(chrROM), newFrameCallback);

    ppu->SetMirroring(rom.MirroringMode());

    auto mapperCode = rom.MapperCode();
    std::cout << "MapperCode: " << mapperCode << std::endl;

    switch (mapperCode) {
    case 0:
        if (prgROMSize > 0x4000) return MakeNESInstance<mapper::NROM256Mapper>(input, ppu, prgROM);
        else                     return MakeNESInstance<mapper::NROM128Mapper>(input, ppu, prgROM);
    case 1:                      return MakeNESInstance<mapper::MMC1Mapper>(input, ppu, prgROM);
    case 2:                      return MakeNESInstance<mapper::UxROMMapper>(input, ppu, prgROM);
    default:                     return MakeNESInstance<mapper::UxROMMapper>(input, ppu, prgROM);
    }
}

} // namespace nemu
