// -----------------------------------------------------------------------------------------* C++ *-
// NESInstance.h
//
// -------------------------------------------------------------------------------------------------

#pragma once

#include "Nemu/Core/CPU.h"
#include "Nemu/Mapper/CPUMapper.h"
#include "Nemu/Mapper/NROM128Mapper.h"
#include "Nemu/Mapper/NROM256Mapper.h"
#include "Nemu/Mapper/MMC1Mapper.h"
#include "Nemu/Mapper/MMC3Mapper.h"
#include "Nemu/Mapper/PPUMapper.h"
#include "Nemu/Mapper/UxROMMapper.h"
#include "Nemu/Core/NESInput.h"
#include "Nemu/Core/PPU.h"
#include "Nemu/Core/ROMLayout.h"
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>
#include <cstdint>

namespace nemu {

class NESInstance {
  public:
    /// Configurations and data used when initailizing a NESInstance.
    struct Descriptor {
        /// The ROM used to load the memory.
        ROMLayout rom;

        /// Input configuration.
        NESInput input;

        /// If set to a value greater than 0 it will be used instead of the mapper given from the 
        /// header.
        int mapperCode;

        // TODO: 
        //   Add configurations such as setting the registers of the CPU, initial state of the PPU,
        //   and so on.
    };

    virtual ~NESInstance() = default;
    virtual void RunFrame() = 0;
    virtual void Power() = 0;
    virtual std::vector<unsigned> DumpCPUMemory() = 0;
	virtual std::unique_ptr<NESInstance> MakeCopy() = 0;
	virtual std::uint8_t* GetPixels() = 0;
};

template <class CartridgeMapper>
class NESInstanceBase: public NESInstance {
    // TODO:
    //  For simplicity everything is shared pointers. There are probably more static solutions.
    std::shared_ptr<mapper::CPUMapper<CartridgeMapper>> cpuMapper;
    std::shared_ptr<mapper::PPUMapper<CartridgeMapper>> ppuMapper;
    std::shared_ptr<CPU<typename decltype(cpuMapper)::element_type>> cpu;
    std::shared_ptr<PPU<typename decltype(ppuMapper)::element_type>> ppu;
	std::shared_ptr<CartridgeMapper> cartridgeMapper;

   public:
    NESInstanceBase(const NESInstance::Descriptor& descriptor)
        : cpu(std::make_shared<typename decltype(cpu)::element_type>())
        , ppu(std::make_shared<typename decltype(ppu)::element_type>())
		, cpuMapper(std::make_shared<typename decltype(cpuMapper)::element_type>())
		, ppuMapper(std::make_shared<typename decltype(ppuMapper)::element_type>())
    {
        auto rom = descriptor.rom;

        auto cartridgeMapper = std::make_shared<CartridgeMapper>(
            std::vector<unsigned>(rom.BeginPRGROM(), rom.EndPRGROM()),
            std::vector<unsigned>(rom.BeginCHRROM(), rom.EndCHRROM()));

		// Connect all objects
		this->cartridgeMapper = cartridgeMapper;

        cartridgeMapper->ppu = ppu;
        cartridgeMapper->cpu = cpu;
        cartridgeMapper->Update();

        cpuMapper->cartridgeMapper = cartridgeMapper;
        cpuMapper->cpu = cpu;
        cpuMapper->ppu = ppu;
        cpuMapper->joypad.SetInput(descriptor.input);
        
        cpu->mapper = cpuMapper;
        ppuMapper->cartridgeMapper = cartridgeMapper;

        ppu->mapper = ppuMapper;
        ppu->SetMirroring(rom.MirroringMode());

        // Set connector lambdas for communication between CPU and PPU.
        ppu->SetNMI = [this]() { this->cpu->SetNMI(); };

        cpu->Tick = [this]() {
            this->ppu->Step();
            this->ppu->Step();
            this->ppu->Step();
            this->cpu->DecrementCycles();
        };
    }

	virtual std::unique_ptr<NESInstance> MakeCopy() override
	{
		return std::make_unique<NESInstanceBase<CartridgeMapper>>(*this);
	}

	virtual std::uint8_t* GetPixels() override
	{
		return ppu->GetPixels();
	}

	NESInstanceBase(const NESInstanceBase<CartridgeMapper>& other)
		: cpuMapper(std::make_shared<typename decltype(cpuMapper)::element_type>(*other.cpuMapper))
		, ppuMapper(std::make_shared<typename decltype(ppuMapper)::element_type>(*other.ppuMapper))
		, cpu(std::make_shared<typename decltype(cpu)::element_type>(*other.cpu))
		, ppu(std::make_shared<typename decltype(ppu)::element_type>(*other.ppu))
		, cartridgeMapper(std::make_shared<CartridgeMapper>(*other.cartridgeMapper))
	{
		// Connect all objects
		cartridgeMapper->ppu = ppu;
		cartridgeMapper->cpu = cpu;

		cpuMapper->cartridgeMapper = cartridgeMapper;
		cpuMapper->cpu = cpu;
		cpuMapper->ppu = ppu;

		ppuMapper->cartridgeMapper = cartridgeMapper;

		cpu->mapper = cpuMapper;
		ppu->mapper = ppuMapper;

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
        cpu->Power();
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
};

template <class CartridgeMapper>
std::unique_ptr<NESInstance> MakeNESInstance(const NESInstance::Descriptor& descriptor)
{ return std::unique_ptr<NESInstance>(new NESInstanceBase<CartridgeMapper>(descriptor)); }

static std::unique_ptr<NESInstance> MakeNESInstance(const NESInstance::Descriptor& descriptor)
{
    auto rom = descriptor.rom;
    std::cout << rom << std::endl;

    switch (descriptor.mapperCode < 0 ? rom.MapperCode() : descriptor.mapperCode) {
    case 0: {
        if (rom.EndPRGROM() - rom.BeginPRGROM() > 0x4000)
            return MakeNESInstance<mapper::NROM256Mapper>(descriptor);
        else
            return MakeNESInstance<mapper::NROM128Mapper>(descriptor);
    }
    case 1:  return MakeNESInstance<mapper::MMC1Mapper>(descriptor);
    case 2:  return MakeNESInstance<mapper::UxROMMapper>(descriptor);
    case 4:  return MakeNESInstance<mapper::MMC3Mapper>(descriptor);
    default: return MakeNESInstance<mapper::UxROMMapper>(descriptor);
    }
    
}

} // namespace nemu