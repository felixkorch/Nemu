#pragma once
#include "Nemu/System.h"
#include "Nemu/Joypad.h"
#include "Nemu/CPU/CPUInterface.h"
#include "Nemu/PPU/PPUInterface.h"
#include <memory>
#include <vector>
#include <cstddef>

namespace nemu 
{
	using namespace cpu;
	using namespace ppu;

	class Nemu {
		static constexpr unsigned TotalCycles = 29781;

		std::unique_ptr<CPUInterface> cpu;
		std::unique_ptr<PPUInterface> ppu;

		std::vector<unsigned> prgRom;
		std::vector<unsigned> prgRam;
		std::vector<unsigned> chrRom;

		Joypad joypad;

	public:

		void LoadROM(const std::string& filePath)
		{
			auto file = ReadFile<std::vector<std::uint8_t>>(filePath);
			std::cout << "File size: " << file.size() << " B" << std::endl;

			std::vector<unsigned> rom;
			for (const auto& byte : file)
				rom.push_back((unsigned)byte);

			if (rom.size() == 0)
				return;

			std::cout << "Rom size: " << rom.size() << " B" << std::endl;

			// Number of prg-blocks in 16KB units
			int prgRomSize = rom[4] * 0x4000;

			// Number of chr-blocks in 8KB units (0 indicates chr-ram)
			int chrRomSize = rom[5] * 0x2000;

			// Upper nybble of flag 7 and 6 represents the mapper version
			int version = (rom[7] & 0xF0) | (rom[6] >> 4);
			MirroringMode mirroringMode = rom[6] & 1 ? MirroringMode::Vertical : MirroringMode::Horizontal;
			SetMirroring(mirroringMode);

			// Number of prgRam-blocks in 8KB units
			int prgRamSize = rom[8] ? rom[8] * 0x2000 : 0x2000;

			auto prgRomBegin = rom.begin() + 16;
			auto chrRomBegin = prgRomBegin + prgRomSize;

			std::cout << "PRG-ROM Size: " << prgRomSize << " B" << std::endl;
			std::cout << "PRG-RAM Size: " << prgRamSize << " B" << std::endl;
			std::cout << "CHR-ROM Size: " << chrRomSize << " B" << std::endl;

			prgRam.resize(prgRamSize);
			prgRom = std::vector<unsigned>(prgRomBegin, chrRomBegin);
			chrRom = std::vector<unsigned>(chrRomBegin, rom.end());
		}

		template <class PPUType, typename ...Args>
		void CreatePPU(Args&& ...args)
		{
			ppu = std::unique_ptr<PPUInterface>(new PPUType(*this, std::forward<Args>(args)...));
		}

		template <class CPUType, typename ...Args>
		void CreateCPU(Args&& ...args)
		{
			cpu = std::unique_ptr<CPUInterface>(new CPUType(*this, std::forward<Args>(args)...));
		}

		void Power()
		{
			cpu->Reset();
		}

		void RunFrame()
		{
			for (int i = 0; i < TotalCycles; i++) {
				cpu->Execute();
				ppu->Step();
				ppu->Step();
				ppu->Step();
			}
		}

		void SetNMI()
		{
			cpu->SetNMI();
		}

		void SetIRQ()
		{
			cpu->SetIRQ();
		}

		void SetMirroring(MirroringMode mode)
		{
			ppu->SetMirroring(mode);
		}

		void PPUWrite(std::size_t index, std::uint8_t value)
		{
			ppu->WriteRegister(index, value);
		}

		std::uint8_t PPURead(std::size_t index)
		{
			return ppu->ReadRegister(index);
		}

		std::uint8_t JoypadRead(unsigned n)
		{
			return joypad.Read(n);
		}

		void JoypadWrite(std::uint8_t value)
		{
			joypad.Write(value);
		}

		void AddJoypad(const Joypad& _joypad)
		{
			joypad = _joypad;
		}

		// Cartrdige access includes no mapping for the moment,
		// Currently only works for NROM(Version 0)
		void CartridgeWritePRG(std::size_t index, unsigned value)
		{
			prgRom[(index - 0x8000) % prgRom.size()] = value;
		}

		unsigned CartridgeReadPRG(std::size_t index)
		{
			return prgRom[(index - 0x8000) % prgRom.size()];
		}

		void CartridgeWriteCHR(std::size_t index, unsigned value)
		{
			chrRom[index % chrRom.size()] = value;
		}

		unsigned CartridgeReadCHR(std::size_t index)
		{
			return chrRom[index % chrRom.size()];
		}

	};



}
