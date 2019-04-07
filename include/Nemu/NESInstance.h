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
#include "Nemu/UxROMMapper.h"
#include "Nemu/PPU.h"
#include "Nemu/System.h"
#include <cstdint>
#include <iostream>
#include <vector>

namespace nemu {

class NESInstance {
  public:
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
        ppu->setNMI = [this]() { this->cpu->SetNMI(); };
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
    // Read file.
    auto file = ReadFile<std::vector<std::uint8_t>>(path);
    std::cout << "File size: " << file.size() << " B" << std::endl;

    std::vector<unsigned> rom;
    for (const auto& byte : file)
        rom.push_back((unsigned)byte);

    // TODO:
    // if (rom.size() == 0)
    //    return;

    // Number of prg-blocks in 16KB units
    int prgRomSize = rom[4] * 0x4000;

    // Number of chr-blocks in 8KB units (0 indicates chr-ram)
    int chrRomSize = rom[5] * 0x2000;

	// If 0 CHR-RAM is present, size will be 8KB
	int chrRamSize = 0x2000;

    // Upper nybble of flag 7 and 6 represents the mapper version
    int version = (rom[7] & 0xF0) | (rom[6] >> 4);
	std::cout << "Mapper version: " << version << std::endl;
    ppu::MirroringMode mirroringMode = rom[6] & 1 ? ppu::MirroringMode::Vertical : ppu::MirroringMode::Horizontal;

    // Number of prgRam-blocks in 8KB units
    int prgRamSize = rom[8] ? rom[8] * 0x2000 : 0x2000;

    auto prgRomBegin = rom.begin() + 16;
    auto chrRomBegin = prgRomBegin + prgRomSize;

    std::cout << "PRG-ROM Size: " << prgRomSize << " B" << std::endl;
    std::cout << "PRG-RAM Size: " << prgRamSize << " B" << std::endl;
    std::cout << "CHR-ROM Size: " << chrRomSize << " B" << std::endl;
	std::cout << "CHR-RAM Size: " << (chrRomSize ? 0 : chrRamSize) << " B" << std::endl;

    std::vector<unsigned> prgROM(prgRomBegin, chrRomBegin);
    std::vector<unsigned> chrROM(chrRomBegin, rom.end());
    std::vector<unsigned> chrRAM(0x2000);

	std::shared_ptr<PPU> ppu;
	if(chrRomSize == 0)
		ppu = std::make_shared<PPU>(std::move(chrRAM), newFrameCallback);
	else
		ppu = std::make_shared<PPU>(std::move(chrROM), newFrameCallback);

    ppu->SetMirroring(mirroringMode);

	std::shared_ptr<NESMapper> mapper;

    switch (version) {
    case 0:
        if (prgRomSize > 0x4000) return MakeNESInstance<NROM256Mapper>(input, ppu, prgROM);
        else                     return MakeNESInstance<NROM128Mapper>(input, ppu, prgROM);
    case 2: return MakeNESInstance<UxROMMapper>(input, ppu, prgROM);
    default: return nullptr;
    }
}

} // namespace nemu
