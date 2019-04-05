// ---------------------------------------------------------------------* C++ *-
// NESInstance.h
//
// -----------------------------------------------------------------------------

#pragma once
#include "Nemu/CPU.h"
#include "Nemu/InternalNESMapper.h"
#include "Nemu/NROM128Mapper.h"
#include "Nemu/NROM256Mapper.h"
#include "Nemu/PPU.h"
#include "Nemu/System.h"
#include <vector>

#include <iostream>

namespace nemu {

class NESInstance {
    std::shared_ptr<CPU<NROM256Mapper>> cpu;
    std::shared_ptr<PPU> ppu;

public:
    NESInstance(std::shared_ptr<CPU<NROM256Mapper>> cpu, std::shared_ptr<PPU> ppu)
        : cpu(cpu)
		, ppu(ppu)
	{
		ppu->setNMI = [this]() { this->cpu->SetNMI(); };
    }

    NESInstance(NESInstance&& other)
        : NESInstance(std::move(other.cpu), other.ppu)
	{}

    void RunFrame()
	{
        for (int i = 0; i < 29781; i++) {
            cpu->Execute();
            ppu->Step();
            ppu->Step();
            ppu->Step();
        }
    }

    void Power()
	{
		cpu->Reset();
	}
};

NESInstance MakeNESInstance(const std::string& path, NESInput& input, std::function<void(std::uint8_t* pixels)> newFrameCallback)
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

    std::cout << "Rom size: " << rom.size() << " B" << std::endl;

    // Number of prg-blocks in 16KB units
    int prgRomSize = rom[4] * 0x4000;

    // Number of chr-blocks in 8KB units (0 indicates chr-ram)
    int chrRomSize = rom[5] * 0x2000;

    // Upper nybble of flag 7 and 6 represents the mapper version
    int version = (rom[7] & 0xF0) | (rom[6] >> 4);
    ppu::MirroringMode mirroringMode = rom[6] & 1 ? ppu::MirroringMode::Vertical : ppu::MirroringMode::Horizontal;

    // TODO:
    // SetMirroring(mirroringMode);

    // Number of prgRam-blocks in 8KB units
    int prgRamSize = rom[8] ? rom[8] * 0x2000 : 0x2000;

    auto prgRomBegin = rom.begin() + 16;
    auto chrRomBegin = prgRomBegin + prgRomSize;

    std::cout << "PRG-ROM Size: " << prgRomSize << " B" << std::endl;
    std::cout << "PRG-RAM Size: " << prgRamSize << " B" << std::endl;
    std::cout << "CHR-ROM Size: " << chrRomSize << " B" << std::endl;

    std::vector<unsigned> prgROM(prgRomBegin, chrRomBegin);
    std::vector<unsigned> chrROM(chrRomBegin, rom.end());

    std::shared_ptr<PPU> ppu(new PPU(std::move(chrROM), newFrameCallback));
    ppu->SetMirroring(mirroringMode);

    InternalNESMapper internalMapper(ppu);
	internalMapper.joypad.AddInputConfig(input);
    std::shared_ptr<CPU<NROM256Mapper>> cpu(new CPU<NROM256Mapper>(std::move(internalMapper), NROM256Mapper(prgROM.cbegin(), prgROM.cend())));

    return NESInstance(cpu, ppu);
}

} // namespace nemu
