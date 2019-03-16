#pragma once
#include "Nemu/NESMemory.h"
#include <cstdint>
#include <cstddef>

namespace nemu
{
	template <class CPUMemory, class PPUMemory>
	class PPU {

	/* Public variables */
	public:
		// The PPU has its own address space: 0x0000 -> 0x3FFF
		// Registers exposed to the CPU
		constexpr static std::uint16_t PPUCTRL   = 0x2000;
		constexpr static std::uint16_t PPUMASK   = 0x2001;
		constexpr static std::uint16_t PPUSTATUS = 0x2002;
		constexpr static std::uint16_t OAMADDR   = 0x2003;
		constexpr static std::uint16_t OAMDATA   = 0x2004;
		constexpr static std::uint16_t PPUSCROLL = 0x2005;
		constexpr static std::uint16_t PPUADDR   = 0x2006;
		constexpr static std::uint16_t PPUDATA   = 0x2007;

		unsigned int nesRGB[] = {
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
		struct ciRAM {
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

		struct AccessOperation {
			std::uint8_t result = 0;
			std::uint8_t buffer = 0;
			bool latch = false;
		};

	/* Member variables */
	private:
		CPUMemory& CPUMem;
		PPUMemory PPUMem;
		AccessOperation access;
		unsigned int pixels[256 * 240];

	/* Public functions */
	public:
		PPU(CPUMemory& cpuMem) :
			CPUMem(cpuMem),
			pixels{}
		{}

	};

} // namespace nemu