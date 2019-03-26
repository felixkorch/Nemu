#pragma once
#include "Nemu/NESMemory.h"
#include <cstdint>
#include <cstddef>
#include <functional>

namespace nemu {
namespace ppu {

	// The PPU has its own address space: 0x0000 -> 0x3FFF
	// Registers exposed to the CPU
    constexpr static unsigned int PPUCTRL   = 0x2000;
    constexpr static unsigned int PPUMASK   = 0x2001;
    constexpr static unsigned int PPUSTATUS = 0x2002;
    constexpr static unsigned int OAMADDR   = 0x2003;
    constexpr static unsigned int OAMDATA   = 0x2004;
    constexpr static unsigned int PPUSCROLL = 0x2005;
    constexpr static unsigned int PPUADDR   = 0x2006;
    constexpr static unsigned int PPUDATA   = 0x2007;

    constexpr static unsigned int nesRGB[] = {
		0x7C7C7C, 0x0000FC, 0x0000BC, 0x4428BC, 0x940084, 0xA80020, 0xA81000, 0x881400,
		0x503000, 0x007800, 0x006800, 0x005800, 0x004058, 0x000000, 0x000000, 0x000000,
		0xBCBCBC, 0x0078F8, 0x0058F8, 0x6844FC, 0xD800CC, 0xE40058, 0xF83800, 0xE45C10,
		0xAC7C00, 0x00B800, 0x00A800, 0x00A844, 0x008888, 0x000000, 0x000000, 0x000000,
		0xF8F8F8, 0x3CBCFC, 0x6888FC, 0x9878F8, 0xF878F8, 0xF85898, 0xF87858, 0xFCA044,
		0xF8B800, 0xB8F818, 0x58D854, 0x58F898, 0x00E8D8, 0x787878, 0x000000, 0x000000,
		0xFCFCFC, 0xA4E4FC, 0xB8B8F8, 0xD8B8F8, 0xF8B8F8, 0xF8A4C0, 0xF0D0B0, 0xFCE0A8,
		0xF8D878, 0xD8F878, 0xB8F8B8, 0xB8F8D8, 0x00FCFC, 0xF8D8F8, 0x000000, 0x000000
	};

	struct Ctrl {
		unsigned int nameTable       = 0; // Nametable ($2000 / $2400 / $2800 / $2C00)
		unsigned int increment       = 0; // Address increment (1 / 32)
		unsigned int spriteTable     = 0; // Sprite pattern table ($0000 / $1000)
		unsigned int backgroundTable = 0; // BG pattern table ($0000 / $1000)
		unsigned int spriteSize      = 0; // Sprite size (8x8 / 8x16)
		unsigned int slave           = 0; // PPU master / slave
		unsigned int nmiEnable       = 0; // Enable NMI
	};

	struct Sprite {
		std::uint8_t id; // Index in OAM.
		std::uint8_t xPos;
		std::uint8_t yPos;
		std::uint8_t tileIndex;
		std::uint8_t attributes;
		std::uint8_t dataLow;
		std::uint8_t dataHigh;
	};

	// PPUSTATUS Register
	struct Status
	{
		unsigned bus       = 0;
		unsigned spriteOvf = 0;  // Sprite overflow
		unsigned spriteHit = 0;  // Sprite 0 Hit
		unsigned vBlank    = 0;  // In VBlank
	};


	// PPUMASK Register
	struct Mask {
		unsigned int gray    = 0; // Grayscale
		unsigned int bgLeft  = 0; // Show background in leftmost 8 pixels
		unsigned int sprLeft = 0; // Show sprite in leftmost 8 pixels
		unsigned int bg      = 0; // Show background
		unsigned int spr     = 0; // Show sprites
		unsigned int red     = 0; // Intensify reds
		unsigned int green   = 0; // Intensify greens
		unsigned int blue    = 0; // Intensify blues
	};

	// PPUADDR Register
	struct Addr {
		unsigned int address;
	};

	enum MirroringMode {
		Vertical, Horizontal
	};

	/*
		The NES has 4 nametables but the board only contains space for 2.
		The nametables takes up the range 0x2000 -> 0x2FFF (4 * 1024 bytes).
		There are 2 mirroring modes -
		Vertical:   0x2000 == 0x2800 && 0x2400 == 0x2C00
		Horizontal: 0x2000 == 0x2400 && 0x2800 == 0x2C00
	*/
	struct CIRAM {
		std::uint8_t memory[2048] = {};
		MirroringMode mirrormode = MirroringMode::Vertical;
		std::uint8_t& operator[](std::size_t offset)
		{
			switch (mirrormode)
			{
			case Vertical:
				return memory[offset % 2048];
			case Horizontal:
				return memory[((offset / 2) & 1024) + (offset % 1024)];
			default:
				return memory[0]; // TODO
			}
		}
	};

	struct PPUInternalMem {
		// CHR-RAM/ROM
		CIRAM        ciRam;      // Holds nametables
		std::uint8_t cgRam[256]; // Holds palettes

		std::uint8_t& operator[](std::size_t offset)
		{
			if (offset > 0x3FFF) { // Out of bounds
				return ciRam[offset]; // Temporary solution
			}
			else if (offset < 0x1FFF) {
				return ciRam[offset]; // TODO: CHR ROM/RAM
			}
			else if (offset < 0x3FFF) {
				return ciRam[offset];
			}
		}
	};

	struct AccessOperation {
		std::uint8_t result = 0;
		std::uint8_t buffer = 0;
		bool latch = false;
	};

	template <class CPUMemory, class PPUMemory>
	class PPU {

    // Member variables
    private:
		std::function<void(std::uint8_t*)> HandleNewFrame;
		CPUMemory& CPUMem;
		PPUMemory PPUMem;
		std::unique_ptr<std::uint8_t[]> pixels; // pixels[ x * height * depth + y * depth + z ] = elements[x][y][z]
		bool newFrame;

		// Background addresses / read operations
		unsigned int VAddr, tempVAddr, fineX;
		AccessOperation access;

		// Background shift registers
		std::uint16_t shiftReg16_1;
		std::uint16_t shiftReg16_2;
		std::uint8_t  shiftReg8_1;
		std::uint8_t  shiftReg8_2;

		// Sprite data / attributes
		unsigned int primaryOAM  [64 * 4]; // 64 sprites, 4 bytes each
		unsigned int secondaryOAM[8 * 4];  // 8 sprites

		int scanline;
		int dot; // Scanline counter
		std::uint16_t fetchAddr;

	public:
		PPU(CPUMemory& cpuMem, std::function<void(std::uint8_t* pixels)> newFrameCallback) :
			CPUMem(cpuMem),
			pixels{},
			HandleNewFrame(newFrameCallback)
		{}

		~PPU()
		{
		}

	private:

		bool isBetween(int x, int a, int b)
		{
			return x >= a && x <= b;
		}

		void ShiftBackground()
		{
			shiftReg16_1 = (shiftReg16_1 & 0xFF00) | shiftReg8_1;
			shiftReg16_2 = (shiftReg16_2 & 0xFF00) | shiftReg8_2;
		}

		void ClearOAM()
		{

		}

		void ScanlinePRE()
		{

		}

		void ScanlineVISIBLE()
		{
			if (dot == 0)
				return;
			else if (dot == 1) {
				ClearOAM();
			}
			else if (isBetween(dot, 2, 255)) { // The data for each tile is fetched
				switch (dot % 8) {
				case 0: {

				}
				case 1: {
					fetchAddr = nt_addr();
					reload_shift();
					break;
				}
				case 2: {

				}
				case 3: {

				}
				case 4: {

				}
				case 5: {

				}
				case 6: {

				}
				case 7: {


				}
				}
			}
		}

		void ScanlinePOST()
		{
			if (dot == 0)
				HandleNewFrame(pixels.get());
		}


	};

} // namespace ppu
} // namespace nemu
