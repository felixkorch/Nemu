#pragma once
#include "Nemu/NESMemory.h"
#include <cstdint>
#include <cstddef>
#include <functional>

namespace nemu {
namespace ppu {

	#define Bit_N(x, n) (((x) >> (n)) & 1)

    constexpr static unsigned nesRGB[] = {
		0x7C7C7C, 0x0000FC, 0x0000BC, 0x4428BC, 0x940084, 0xA80020, 0xA81000, 0x881400,
		0x503000, 0x007800, 0x006800, 0x005800, 0x004058, 0x000000, 0x000000, 0x000000,
		0xBCBCBC, 0x0078F8, 0x0058F8, 0x6844FC, 0xD800CC, 0xE40058, 0xF83800, 0xE45C10,
		0xAC7C00, 0x00B800, 0x00A800, 0x00A844, 0x008888, 0x000000, 0x000000, 0x000000,
		0xF8F8F8, 0x3CBCFC, 0x6888FC, 0x9878F8, 0xF878F8, 0xF85898, 0xF87858, 0xFCA044,
		0xF8B800, 0xB8F818, 0x58D854, 0x58F898, 0x00E8D8, 0x787878, 0x000000, 0x000000,
		0xFCFCFC, 0xA4E4FC, 0xB8B8F8, 0xD8B8F8, 0xF8B8F8, 0xF8A4C0, 0xF0D0B0, 0xFCE0A8,
		0xF8D878, 0xD8F878, 0xB8F8B8, 0xB8F8D8, 0x00FCFC, 0xF8D8F8, 0x000000, 0x000000
	};

	enum MirroringMode {
		Vertical, Horizontal
	};

	template <class CPUType>
	class PPU {

		///	The NES has 4 nametables but the board only contains space for 2.
		///	The nametables takes up the range 0x2000 -> 0x2FFF (4 * 1024 bytes).
		///	There are 2 mirroring modes -
		///	Vertical:   0x2000 == 0x2800 && 0x2400 == 0x2C00
		///	Horizontal: 0x2000 == 0x2400 && 0x2800 == 0x2C00

        class CiRam {
            std::uint8_t memory[2048] = {};
			MirroringMode mirrormode = MirroringMode::Vertical;
            PPU& ppuRef;
        public:
            CiRam(PPU& ppuRef)
                : ppuRef(ppuRef) {}

            std::uint8_t& operator[](std::size_t offset)
			{
                switch (mirrormode) {
				case Vertical:
					return memory[offset % 2048];
				case Horizontal:
					return memory[((offset / 2) & 1024) + (offset % 1024)];
				default:
                    return ppuRef.addr - 0x2000;
				}
			}
		};

        class InternalMemory {
            PPU& ppuRef;
            CiRam        ciRam;      // Holds nametables
            std::uint8_t cgRam[256]; // Holds palettes
        public:
            InternalMemory(PPU& ppuRef)
                : ppuRef(ppuRef), ciRam(ppuRef) {}

            std::uint8_t& operator [](std::size_t index)
			{
                if (index <= 0x1FFF) {  // CHR-ROM/RAM.
                    return ppuRef.cpuRef.GetMemory()[index];
                }
                if (index <= 0x3EFF) { // Nametables.
                    return ciRam[index];
                }
                if (index <= 0x3FFF) { // Palettes.
                    if ((ppuRef.addr & 0x13) == 0x10) ppuRef.addr &= ~0x10;
                    return cgRam[index & 0x1F];
                }
                return cgRam[0]; // Out of bounds.
			}

            std::uint8_t Read(std::size_t index)
            {
                return *this[index] & (ppuRef.mask.gray ? 0x30 : 0xFF);
            }
		};

        union Ctrl {
            struct {
                unsigned nameTable       : 2; // Nametable ($2000 / $2400 / $2800 / $2C00)
                unsigned increment       : 1; // Address increment (1 / 32)
                unsigned spriteTable     : 1; // Sprite pattern table ($0000 / $1000)
                unsigned backgroundTable : 1; // BG pattern table ($0000 / $1000)
                unsigned spriteSize      : 1; // Sprite size (8x8 / 8x16)
                unsigned slave           : 1; // PPU master / slave
                unsigned nmiEnable       : 1; // Enable NMI
            };
            std::uint8_t reg;
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
        union Status {
            struct {
                unsigned bus       : 5;
                unsigned spriteOvf : 1;  // Sprite overflow
                unsigned spriteHit : 1;  // Sprite 0 Hit
                unsigned vBlank    : 1;  // In VBlank
            };
            std::uint8_t reg;
        };


        // PPUMASK Register
        union Mask {
            struct {
                unsigned gray    : 1; // Grayscale
                unsigned bgLeft  : 1; // Show background in leftmost 8 pixels
                unsigned sprLeft : 1; // Show sprite in leftmost 8 pixels
                unsigned bg      : 1; // Show background
                unsigned spr     : 1; // Show sprites
                unsigned red     : 1; // Intensify reds
                unsigned green   : 1; // Intensify greens
                unsigned blue    : 1; // Intensify blues
            };
            std::uint8_t reg;
        };

        // PPUADDR Register
        union Addr {
            struct {
                unsigned coarseX   : 5; // Coarse X.
                unsigned coarseY   : 5; // Coarse Y.
                unsigned nametable : 2; // Nametable.
                unsigned fineY     : 3; // Fine Y.
            };
            struct
            {
                unsigned low  : 8;
                unsigned high : 7;
            };
            unsigned addr : 14;
            unsigned reg : 15;
        };

        struct AccessOperation {
            std::uint8_t result = 0;
            std::uint8_t buffer = 0;
            bool latch = false;
        };

        // Member variables
		std::function<void(std::uint8_t*)> HandleNewFrame;
		CPUType cpuRef;
        InternalMemory internalMemory;
		std::unique_ptr<std::uint8_t[256 * 240]> pixels;

		Ctrl ctrl;
		Mask mask;
		Status status;

		// Background addresses / read operations

		Addr vAddr, vAddrTemp;
		std::uint8_t oamAddr;
		std::uint8_t fineX;
		AccessOperation access;
        MirroringMode mirroring;

		// Background shift registers
		std::uint16_t bgShiftLow;
		std::uint16_t bgShiftHigh;
		std::uint8_t  atShiftLow;
		std::uint8_t  atShiftHigh;
        bool atLatchLow, atLatchHigh;
		// Background latches:
		std::uint8_t nt, at, bgL, bgH;

		// Sprite data / attributes
		Sprite primaryOam  [8];
		Sprite secondaryOam[8];
		std::uint8_t oamMem[256];

		int scanline; // Rows
		int dot;      // Columns
		bool frameOdd;
		std::uint16_t addr;

	public:
		PPU(CPUType& cpu, std::function<void(std::uint8_t* pixels)> newFrameCallback) :
			HandleNewFrame(newFrameCallback),
			cpuRef(cpu),
            internalMemory(*this),
			pixels{},
			ctrl{},
			vAddr{},
			vAddrTemp{},
			access{},
			mask{},
			status{},
			bgShiftLow(0),
			bgShiftHigh(0),
			atShiftLow(0),
			atShiftHigh(0),
			nt(0),
			at(0),
			bgL(0),
			bgH(0),
			primaryOam{},
			secondaryOam{},
			oamMem{},
			scanline(0),
			dot(0),
			addr(0),
			frameOdd(false) {}

		~PPU() {}

        class PPURegister {
        public:
            std::uint8_t& operator[](std::size_t index)
            {
                return Read
            }
        };

        std::uint8_t ReadRegister(std::size_t index)
		{
			switch (index) {
			// PPUSTATUS ($2002):
			case 2: {
				access.result = (access.result & 0x1F) | status.reg;
				status.vBlank = 0;
				access.latch = 0;
				break;
			}
			// OAMDATA ($2004).
			case 4: {
				access.result = primaryOam[oamAddr];
				break;
			}
			// PPUDATA ($2007).
			case 7:                                 
				if (vAddr.addr <= 0x3EFF) {
					access.result = access.buffer;
                    access.buffer = internalMemory.Read(vAddr.addr);
				}
				else {
                    access.result = access.buffer = internalMemory.Read(vAddr.addr);
				}
				vAddr.addr += ctrl.increment ? 32 : 1;
			}
			return access.result;
		}

        void WriteRegister(std::size_t index, std::uint8_t value)
		{
			access.result = value;

			switch (index) {
			// PPUCTRL   ($2000).
			case 0: {
				ctrl.reg = value;
				vAddrTemp.nametable = ctrl.nameTable;
				break;
			}
			// PPUMASK   ($2001).
			case 1: {
                mask.reg = value;
				break;
			}
			// OAMADDR   ($2003).
			case 3: {
				oamAddr = value;
				break;
			}
			// OAMDATA   ($2004).
			case 4: {
				primaryOam[oamAddr++] = value;
				break;
			}
			// PPUSCROLL ($2005).
			case 5: {
				// First write.
				if (!access.latch) {
					fineX = value & 7;
					vAddrTemp.coarseX = value >> 3;
				} 
				// Second write.
				else {
					vAddrTemp.fineY = value & 7;
					vAddrTemp.coarseY = value >> 3;
				}
				access.latch = !access.latch;
				break;
			}
			// PPUADDR   ($2006).
			case 6: {
				// First write.
				if (!access.latch) {
                    vAddrTemp.high = value & 0x3F;
				} 
				// Second write.
				else {
                    vAddrTemp.low = value;
					vAddrTemp.reg = vAddrTemp.reg;
				}
				access.latch = !access.latch;
				break;
			}
			// PPUDATA ($2007).
			case 7: {
                internalMemory[vAddr.addr] = value;
				vAddr.addr += ctrl.increment ? 32 : 1;
			}
			}
		}

	private:

		std::uint16_t NametableAddress()      { return 0x2000 | (vAddr.reg & 0xFFF); }
		std::uint16_t AttributeTableAddress() { return 0x23C0 | (vAddr.nametable << 10) | ((vAddr.coarseY / 4) << 3) | (vAddr.coarseX / 4); }
		std::uint16_t BackgroundAddress()     { return (ctrl.backgroundTable * 0x1000) + (vAddr.nametable * 16) + vAddr.fineY; }
		bool Rendering() { return mask.bg || mask.spr; }
		int SpriteHeight() { return ctrl.spriteSize ? 16 : 8; }
		void SetMirroring(MirroringMode mode) { mirroring = mode; }


		void ShiftBackground()
		{
			bgShiftLow = (bgShiftLow & 0xFF00) | bgL;
			bgShiftHigh = (bgShiftHigh & 0xFF00) | bgH;
			atLatchLow = (at & 1);
			atLatchHigh = (at & 2);
		}

		void ClearOAM()
		{
			for (int i = 0; i < 8; i++)
			{
				secondaryOam[i].id = 64;
				secondaryOam[i].y = 0xFF;
				secondaryOam[i].tile = 0xFF;
				secondaryOam[i].attr = 0xFF;
				secondaryOam[i].x = 0xFF;
				secondaryOam[i].dataL = 0;
				secondaryOam[i].dataH = 0;
			}
		}

		void EvalSprites()
		{
			int n = 0;
			for (int i = 0; i < 64; i++) {
				int line = (scanline == 261 ? -1 : scanline) - primaryOam[i * 4 + 0];
				// If the sprite is in the scanline, copy its properties into secondary OAM:
				if (line >= 0 and line < SpriteHeight())
				{
					secondaryOam[n].id         = i;
					secondaryOam[n].yPos       = primaryOam[i * 4 + 0];
					secondaryOam[n].tileIndex  = primaryOam[i * 4 + 1];
					secondaryOam[n].attributes = primaryOam[i * 4 + 2];
					secondaryOam[n].xPos       = primaryOam[i * 4 + 3];

					if (++n > 8)
					{
						status.spriteOvf = true;
						break;
					}
				}
			}
		}

		/* Load the sprite info into primary OAM and fetch their tile data. */
		void LoadSprites()
		{
			std::uint16_t addr;
			for (int i = 0; i < 8; i++)
			{
				primaryOam[i] = secondaryOam[i];  // Copy secondary OAM into primary.

				// Different address modes depending on the sprite height:
				if (SpriteHeight() == 16)
					addr = ((primaryOam[i].tileIndex & 1) * 0x1000) + ((primaryOam[i].tileIndex & ~1) * 16);
				else
					addr = (ctrl.spriteTable * 0x1000) + (primaryOam[i].tileIndex * 16);

                unsigned sprY = (scanline - primaryOam[i].yPos) % SpriteHeight();  // Line inside the sprite.
				if (primaryOam[i].attributes & 0x80) sprY ^= SpriteHeight() - 1;   // Vertical flip.
				addr += sprY + (sprY & 8);  // Select the second tile if on 8x16.

				primaryOam[i].dataLow = rd(addr + 0);
				primaryOam[i].dataHigh = rd(addr + 8);
			}
		}

		// Evaluates if a pixel should be drawn
		void Pixel()
		{
			std::uint8_t palette = 0, objPalette = 0;
			bool objPriority = false;
			int x = dot - 2;

			if (scanline < 240 && x >= 0 && x < 256)
			{
				if (mask.bg && (mask.bgLeft || x >= 8))
				{
					// Background:
					palette = (Bit_N(bgShiftHigh, 15 - fineX) << 1) | Bit_N(bgShiftLow, 15 - fineX);
					if (palette)
						palette |= ((Bit_N(atShiftHigh, 7 - fineX) << 1) | Bit_N(atShiftLow, 7 - fineX)) << 2;
				}
				// Sprites:
				if (mask.spr && (mask.sprLeft || x >= 8))
					for (int i = 7; i >= 0; i--) {
						if (primaryOam[i].id == 64)
							continue;

                        unsigned sprX = x - primaryOam[i].xPos;
						if (sprX >= 8) // Not in range.
							continue;

						if (primaryOam[i].attributes & 0x40) // Horizontal flip.
							sprX ^= 7;

						std::uint8_t sprPalette = (Bit_N(primaryOam[i].dataHigh, 7 - sprX) << 1) | Bit_N(primaryOam[i].dataLow , 7 - sprX);

						if (sprPalette == 0) // Transparent pixel.
							continue;

						if (primaryOam[i].id == 0 && palette && x != 255)
                            status.spriteHit = true;

						sprPalette |= (primaryOam[i].attr & 3) << 2;
						objPalette = sprPalette + 16;
						objPriority = primaryOam[i].attr & 0x20;
					}
				// Evaluate priority:
				if (objPalette && (palette == 0 || objPriority == 0))
					palette = objPalette;

                pixels[scanline * 256 + x] = nesRGB(internalMemory.Read(0x3F00 + (rendering() ? palette : 0)));
			}
			// Perform background shifts:
			bgShiftLow <<= 1; bgShiftHigh <<= 1;
            atShiftLow = (atShiftLow << 1) | atLatchLow;
            atShiftHigh = (atShiftHigh << 1) | atLatchHigh;
		}


		void ScanlinePRE()
		{
			status = status.spriteHit = false;
			ScanlineVISIBLE();
			//if (dot >= 280 && dot <= 304) {
				// TODO: v_update();
			//}
			if(dot == 1)
                status.vBlank = false;

			if (Rendering() && frameOdd)
				dot++;
		}

		void ScanlineVISIBLE()
		{
			if (dot == 1) {
				ClearOAM();
			}
			else if (dot == 257) {
				EvalSprites();
			}
			else if (dot == 321) {
				LoadSprites();
			}

			if ((dot >= 2 && dot <= 255) || (dot >= 322 && dot <= 337)) { // The data for each tile is fetched
				Pixel();
				switch (dot % 8) {

				// Nametables
				case 1: {
					addr = NametableAddress();
					ShiftBackground();
					break;
				}
				case 2: {
                    nt = internalMemory.Read(addr);
					break;
				}
				// Attributes
				case 3: {
					addr = AttributeTableAddress();
				}
				case 4: {
                    at = internalMemory.Read(addr);
					if (vAddr.coarseY & 2) at >>= 4;
					if (vAddr.coarseX & 2) at >>= 2;
					break;
				}
				// Background Low
				case 5: {
					addr = BackgroundAddress();
					break;
				}
				case 6: {
                    bgL = internalMemory.Read(addr);
					break;
				}
				// Background High
				case 7: {
					addr += 8;
					break;
				}
				case 0: {
                    bgH = internalMemory.Read(addr);
					// TODO: h_scroll();
					break;
				}
				}
			}

			if (dot == 256) {
				Pixel();
                bgH = internalMemory.Read(addr); // TODO: v_scroll();
			}
			else if (dot == 257) { // Update horizontal position.
				Pixel();
				ShiftBackground();
				// TODO: h_update(); (Scrolling)
			}

			if (dot == 1) {
				addr = NametableAddress();
			}
			else if (dot >= 321 && dot <= 339) {
				addr = NametableAddress();
			}
			else if (dot == 338) {
                nt = internalMemory.Read(addr);
			}
			else if (dot == 340) {
                nt = internalMemory.Read(addr);
			}

			// Signal scanline to mapper:
			// if (dot == 260 && rendering()) Cartridge::signal_scanline(); Big question mark
		}

		void ScanlinePOST()
		{
			if (dot == 0)
				HandleNewFrame(pixels.get());
		}

		void ScanlineNMI()
		{
			if (dot == 1) {
				status.vBlank = true;
				if (ctrl.nmiEnable)
					cpuRef.InvokeNMI();
			}
		}

		void Step()
		{
			if (scanline >= 0 && scanline <= 239) {
				ScanlineVISIBLE();
			}
			else if (scanline == 240) {
				ScanlinePOST();
			}
			else if (scanline == 241) {
				ScanlineNMI();
			}
			else if (scanline == 261) {
				ScanlinePRE();
			}

			// Update dot and scanline counters:
			if (++dot > 340)
			{
				dot %= 341;
				if (++scanline > 261)
				{
					scanline = 0;
					frameOdd = !frameOdd;
				}
			}
		}

		void Reset()
		{
			frameOdd = false;
			scanline = dot = 0;
			ctrl.reg = mask.reg = status.reg = 0;
			access = AccessOperation{};
            memset(pixels, 0x00, sizeof(pixels));
			memset(oamMem, 0x00, sizeof(oamMem));
            internalMemory = InternalMemory(*this);
		}

	};

} // namespace ppu
} // namespace nemu
