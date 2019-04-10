// ---------------------------------------------------------------------* C++ *-
// NESInstance.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/CPU.h"
#include "Nemu/InternalNESMapper.h"
#include "Nemu/NESCPUMemoryMapper.h"
#include "Nemu/NROM128Mapper.h"
#include "Nemu/NROM256Mapper.h"
#include "Nemu/PPU.h"
#include "Nemu/ROMLayout.h"
#include "Nemu/UxROMMapper.h"
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
    virtual void AddInput(const NESInput& input) = 0;
};

template <class CartridgePRGROM>
class NESInstanceBase: public NESInstance {
    using CPUMemory = NESCPUMemoryMapper<CartridgePRGROM>;
    using CPU = CPU<CPUMemory>;

    // TODO:
    //  For simplicity everything is shared pointers. There are probably more static solutions.
    std::shared_ptr<CPUMemory> cpuMemory;
    std::shared_ptr<CPU> cpu;
    std::shared_ptr<PPU> ppu;

  public:
    NESInstanceBase(std::shared_ptr<PPU> ppu, std::shared_ptr<CartridgePRGROM> prgROM)
        : ppu(ppu)
    {
        // Create the memory of the CPU.
        cpuMemory = std::make_shared<CPUMemory>();
        cpuMemory->internalMapper = std::make_shared<InternalNESMapper<CPU>>();
        cpuMemory->cartridgeMapper = prgROM;

        // Create the CPU.
        cpu = std::make_shared<CPU>(cpuMemory);

        // Connect CPU and PPU to the internal CPU memory.
        cpuMemory->internalMapper->cpu = cpu;
        cpuMemory->internalMapper->ppu = ppu;

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
    {
        cpu->RunFrame();
    }

    virtual void Power() override
    {
        cpu->Reset();
        ppu->Reset();
    }

    /// Placeholder idea for future implementation of save state and debugging tools.
    virtual std::vector<unsigned> DumpCPUMemory() override
    {
        std::vector<unsigned> memory(0xFFFF);
        for(std::size_t i = 0; i < 0xFFFF; ++i)
            memory[i] = cpuMemory->Read(i);
        return memory;
    }

    virtual void AddInput(const NESInput& input) override
    {
        cpuMemory->internalMapper->joypad.AddInputConfig(input);
    }
};

template <class CartridgePRGROM>
std::unique_ptr<NESInstanceBase<CartridgePRGROM>>
MakeNESInstance(const NESInput& input, std::shared_ptr<PPU> ppu,
                const std::vector<unsigned>& prgROM)
{
    std::unique_ptr<NESInstanceBase<CartridgePRGROM>> instance(new NESInstanceBase<CartridgePRGROM>(
        ppu, std::make_shared<CartridgePRGROM>(prgROM.cbegin(), prgROM.cend())));
    instance->AddInput(input);
    return instance;
}

std::unique_ptr<NESInstance>
MakeNESInstance(const std::string& path,
                NESInput& input,
                std::function<void(std::uint8_t* pixels)> newFrameCallback)
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
        if (prgROMSize > 0x4000) return MakeNESInstance<NROM256Mapper>(input, ppu, prgROM);
        else                     return MakeNESInstance<NROM128Mapper>(input, ppu, prgROM);
    case 2: return MakeNESInstance<UxROMMapper>(input, ppu, prgROM);
    default: return MakeNESInstance<UxROMMapper>(input, ppu, prgROM);;
    }
}

} // namespace nemu
