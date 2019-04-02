#pragma once
#include "Nemu/Common.h"
#include "Nemu/Nemu.h"
#include "Nemu/NESMemory.h"
#include <cstdint>
#include <cstddef>
#include <functional>

namespace nemu {
	namespace ppu {

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

	struct RGBA {
		std::uint8_t x, y, z, w;
	};

	RGBA HexToRGBA(unsigned hexValue)
	{
		RGBA rgbaColor;

		rgbaColor.x = ((hexValue >> 16) & 0xFF);
		rgbaColor.y = ((hexValue >> 8) & 0xFF);
		rgbaColor.z = ((hexValue) & 0xFF);
		rgbaColor.w = 255;

		return rgbaColor;
	}

    class PPU : public PPUInterface {

		///	The NES has 4 nametables but the board only contains space for 2.
		///	The nametables takes up the range 0x2000 -> 0x2FFF (4 * 1024 bytes).
		///	There are 2 mirroring modes -
		///	Vertical:   0x2000 == 0x2800 && 0x2400 == 0x2C00
		///	Horizontal: 0x2000 == 0x2400 && 0x2800 == 0x2C00

        class CiRam {
            std::uint8_t memory[0x800];
			MirroringMode mirrormode;
        public:
            CiRam(PPU& ppu) :
				memory{},
				mirrormode(MirroringMode::Vertical)
			{}

			void SetMirroring(MirroringMode mode)
			{
				mirrormode = mode;
			}

			std::uint8_t Read(std::size_t index)
			{
				switch (mirrormode) {
				case Vertical:
					return memory[index % 0x800];
				case Horizontal:
					return memory[((index / 2) & 0x400) + (index % 0x400)];
				default:
					return index - 0x2000;
				}
			}

			void Write(std::size_t index, std::uint8_t value)
			{
				switch (mirrormode) {
				case Vertical:
					memory[index % 0x800] = value;
				case Horizontal:
					memory[((index / 2) & 0x400) + (index % 0x400)] = value;
				}
			}
		};

        class InternalMemory {
			std::reference_wrapper<PPU> ppu;
            CiRam        ciRam;       // Holds nametables (2048 B)
            std::uint8_t cgRam[0x20]; // Holds palettes (32 B)
        public:
            InternalMemory(PPU& ppu) :
				ppu(ppu),
				ciRam(ppu),
				cgRam{}
			{}

			void SetMirroring(MirroringMode mode)
			{
				ciRam.SetMirroring(mode);
			}

			void Write(std::size_t index, std::uint8_t value)
			{
				if (index <= 0x1FFF) {  // CHR-ROM/RAM.
					ppu.get().nemu.CartridgeWriteCHR(index, value);
					return;
				}
				if (index <= 0x3EFF) { // Nametables.
					ciRam.Write(index, value);
					return;
				}
				if (index <= 0x3FFF) { // Palettes.
					if ((index & 0x13) == 0x10)
						index &= ~0x10;
					cgRam[index & 0x1F] = value;
					return;
				}
			}

            std::uint8_t Read(std::size_t index)
            {
				if (index <= 0x1FFF) {  // CHR-ROM/RAM.
					return ppu.get().nemu.CartridgeReadCHR(index);
				}
				if (index <= 0x3EFF) { // Nametables.
					return ciRam.Read(index);
				}
				if (index <= 0x3FFF) { // Palettes.
					if ((index & 0x13) == 0x10)
						index &= ~0x10;
					return cgRam[index & 0x1F] & (ppu.get().mask.gray ? 0x30 : 0xFF);
				}
				return 0; // Out of bounds.
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
                unsigned red     : 1; // Emphasize red
                unsigned green   : 1; // Emphasize green
                unsigned blue    : 1; // Emphasize blue
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
	private:

		Nemu& nemu;
		std::function<void(std::uint8_t*)> HandleNewFrame;
        InternalMemory internalMemory;
		std::unique_ptr<std::uint8_t[]> pixels;

		Ctrl ctrl;
		Mask mask;
		Status status;

		Addr vAddr, tAddr;
		std::uint8_t oamAddr;
		std::uint8_t fineX;
		AccessOperation access;

		// Background shift registers
		std::uint16_t bgShiftLow, bgShiftHigh;
		std::uint8_t  atShiftLow, atShiftHigh;
		std::uint8_t nt, at, bgL, bgH;
        bool atLatchLow, atLatchHigh;

		// Sprite data / attributes
		Sprite oam [8];
		Sprite secondaryOam[8];
		std::uint8_t oamMem[0x100]; // 256 B (64 sprites * 4B each)

		int scanline; // Rows
		int dot;      // Columns
		bool frameOdd;

	public:
        PPU(Nemu& nemu, std::function<void(std::uint8_t* pixels)> newFrameCallback) :
			nemu(nemu),
			HandleNewFrame(newFrameCallback),
			internalMemory(*this)
		{
			Reset();
		}

		~PPU() {}

		virtual void Reset() override
		{
			internalMemory = InternalMemory(*this);
			pixels = std::unique_ptr<std::uint8_t[]>(new std::uint8_t[256 * 240 * 4]);
			ctrl = {};
			mask = {};
			vAddr = tAddr = {};
			oamAddr = fineX = 0;
			access = {};
			bgShiftLow = bgShiftHigh = atShiftLow = atLatchHigh = nt = at = bgL = bgH = 0;
			atLatchLow = atLatchHigh = false;
			memset(oam, 0, sizeof(oam));
			memset(secondaryOam, 0, sizeof(secondaryOam));
			memset(oamMem, 0, sizeof(oamMem));
			scanline = dot = 0;
			frameOdd = false;
		}

        virtual std::uint8_t ReadRegister(std::size_t index) override
		{
			switch (index) {
			case 2: { // PPUSTATUS ($2002)
				access.result = (access.result & 0x1F) | status.reg;
				status.vBlank = 0;
				access.latch = false;
				break;
			}
			case 4: { // OAMDATA ($2004)
				access.result = oamMem[oamAddr];
				break;
			}
			case 7: { // PPUDATA ($2007)                     
				if (vAddr.addr <= 0x3EFF) {
					access.result = access.buffer;
					access.buffer = internalMemory.Read(vAddr.addr);
				}
				else {
					access.result = access.buffer = internalMemory.Read(vAddr.addr);
				}
				vAddr.addr += ctrl.increment ? 32 : 1;
			}
			}
			return access.result;
		}

        virtual void WriteRegister(std::size_t index, std::uint8_t value) override
		{
			access.result = value;

			switch (index) {
			case 0: { // PPUCTRL ($2000)
				ctrl.reg = value;
				tAddr.nametable = ctrl.nameTable;
				break;
			}
			case 1: { // PPUMASK ($2001)
                mask.reg = value;
				break;
			}
			case 3: { // OAMADDR ($2003)
				oamAddr = value;
				break;
			}
			case 4: { // OAMDATA ($2004)
				oamMem[oamAddr++] = value;
				break;
			}
			case 5: { // PPUSCROLL ($2005)
				if (!access.latch) {            // First write
					fineX = value & 7;
					tAddr.coarseX = value >> 3;
				} 
				else {                          // Second write
					tAddr.fineY = value & 7;
					tAddr.coarseY = value >> 3;
				}
				access.latch = !access.latch;
				break;
			}
			case 6: { // PPUADDR ($2006)
				// First write
				if (!access.latch) {
                    tAddr.high = value & 0x3F;
				} 
				// Second write
				else {
                    tAddr.low = value;
					vAddr.reg = tAddr.reg;
				}
				access.latch = !access.latch;
				break;
			}
			case 7: { // PPUDATA ($2007)
                internalMemory.Write(vAddr.addr, value);
				vAddr.addr += ctrl.increment ? 32 : 1;
			}
			}
		}

		virtual void Step() override
		{
			if (scanline >= 0 && scanline <= 239) {
				Scanline(ScanlinePhase::VISIBLE);
			}
			else if (scanline == 240) {
				Scanline(ScanlinePhase::POST);
			}
			else if (scanline == 241) {
				Scanline(ScanlinePhase::NMI);
			}
			else if (scanline == 261) {
				Scanline(ScanlinePhase::PRE);
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

		virtual void SetMirroring(MirroringMode mode) override
		{
			internalMemory.SetMirroring(mode);
		}


	private:

		std::uint16_t NametableAddress()      { return 0x2000 | (vAddr.reg & 0xFFF); }
		std::uint16_t AttributeTableAddress() { return 0x23C0 | (vAddr.nametable << 10) | ((vAddr.coarseY / 4) << 3) | (vAddr.coarseX / 4); }
		std::uint16_t BackgroundAddress()     { return (ctrl.backgroundTable * 0x1000) + (nt * 16) + vAddr.fineY; }
		bool IsRendering() { return mask.bg || mask.spr; }
		int SpriteHeight() { return ctrl.spriteSize ? 16 : 8; }
		void UpdateScrollH() { if (!IsRendering()) return; vAddr.reg = (vAddr.reg & ~0x041F) | (tAddr.reg & 0x041F); }
		void UpdateScrollV() { if (!IsRendering()) return; vAddr.reg = (vAddr.reg & ~0x7BE0) | (tAddr.reg & 0x7BE0); }

		void ScrollH()
		{
			if (!IsRendering())
				return;
			if (vAddr.coarseX == 31)
				vAddr.reg ^= 0x41F;
			else
				vAddr.coarseX++;
		}

		void ScrollV()
		{
			if (!IsRendering())
				return;
			if (vAddr.fineY < 7) {
				vAddr.fineY++;
			}
			else {
				vAddr.fineY = 0;
				if (vAddr.coarseY == 31) vAddr.coarseY = 0;
				else if (vAddr.coarseY == 29) {
					vAddr.coarseY = 0;
					vAddr.nametable ^= 0b10;
				}
				else
					vAddr.coarseY++;
			}
		}

		void ShiftBackground()
		{
			bgShiftLow  = (bgShiftLow & 0xFF00)  | bgL;
			bgShiftHigh = (bgShiftHigh & 0xFF00) | bgH;
			atLatchLow  = (at & 1);
			atLatchHigh = (at & 2);
		}

		void ClearOAM()
		{
			for (int i = 0; i < 8; i++)
			{
				secondaryOam[i].id         = 64;
				secondaryOam[i].yPos       = 0xFF;
				secondaryOam[i].tileIndex  = 0xFF;
				secondaryOam[i].attributes = 0xFF;
				secondaryOam[i].xPos       = 0xFF;
				secondaryOam[i].dataLow    = 0;
				secondaryOam[i].dataHigh   = 0;
			}
		}

		void EvalSprites()
		{
			// Dummy scanline
			if (scanline == 261)
				return;

			// Number of sprites in the scanline
			unsigned count = 0;

			for (unsigned i = 0; i < 64; i++) {
				const int diff = scanline - oamMem[i * 4 + 0];
				// Checks if the sprite is on the scanline
				if (diff >= 0 && diff < SpriteHeight()) {
					secondaryOam[count].id = i;
					secondaryOam[count].yPos = oamMem[i * 4 + 0];
					secondaryOam[count].tileIndex = oamMem[i * 4 + 1];
					secondaryOam[count].attributes = oamMem[i * 4 + 2];
					secondaryOam[count].xPos = oamMem[i * 4 + 3];

					// Maximum of 8 sprites is possible
					if (++count > 8) {
						status.spriteOvf = true;
						return;
					}
				}
			}
		}

		void LoadSprites()
		{
			unsigned addr;
			for (unsigned i = 0; i < 8; i++) {
				oam[i] = secondaryOam[i];

				// 8 x 16
				if (SpriteHeight() == 16) {
					// Bit 0 selects either Pattern table 0 or 1 (0x0000 / 0x1000)
					const unsigned table = oam[i].tileIndex & Bit0;
					// The 7 MSB represents the index (Each tile is 16 bytes)
					const unsigned index = (oam[i].tileIndex & ~Bit0) * 16;
					addr = (table ? 0x1000 : 0x0000) + index;
				}
				// 8 x 8
				else {
					// Table selected by bit 3 of the CTRL register
					const unsigned table = ctrl.spriteTable;
					// For 8x8 sprites the index is simply the byte
					const unsigned index = oam[i].tileIndex * 16;
					addr = (table ? 0x1000 : 0x0000) + index;
				}

				unsigned sprY = (scanline - oam[i].yPos) % SpriteHeight();  // Line inside the sprite

				// Bit 7 indicates vertical flip
				if (oam[i].attributes & Bit7)
					sprY ^= SpriteHeight() - 1;

				// Select the second tile if on 8x16
				addr += sprY + (sprY & 8);

				oam[i].dataLow  = internalMemory.Read(addr + 0);
				oam[i].dataHigh = internalMemory.Read(addr + 8);

			}
		}

		// Evaluates which pixel should be drawn
		void Pixel()
		{
			std::uint8_t palette = 0, objPalette = 0;
			bool objPriority = false;
			int x = dot - 2;

			if (scanline < 240 && x >= 0 && x < 256) {
				if (mask.bg && !(!mask.bgLeft && x < 8)) {
					// Background:
					palette = (NthBit(bgShiftHigh, 15 - fineX) << 1) |
						       NthBit(bgShiftLow, 15 - fineX);
					if (palette)
						palette |= ((NthBit(atShiftHigh, 7 - fineX) << 1) |
							         NthBit(atShiftLow, 7 - fineX)) << 2;
				}
				// Sprites:
				if (mask.spr && !(!mask.sprLeft && x < 8)) {
					for (int i = 7; i >= 0; i--) {
						if (oam[i].id == 64)
							continue;

						// If the sprite is outside of the tile, it should not be drawn
						unsigned sprX = x - oam[i].xPos;
						if (sprX >= 8)
							continue;

						// Bit 6 indicates horizontal flip
						if (oam[i].attributes & Bit6)
							sprX ^= 7;

						std::uint8_t sprPalette = (NthBit(oam[i].dataHigh, 7 - sprX) << 1) |
							                       NthBit(oam[i].dataLow, 7 - sprX);

						if (sprPalette == 0) // Transparent pixel.
							continue;

						if (oam[i].id == 0 && palette && x != 255)
							status.spriteHit = true;

						sprPalette |= (oam[i].attributes & 3) << 2;
						objPalette  = sprPalette + 16;
						objPriority = oam[i].attributes & 0x20;
					}
				}
				// Evaluate priority:
				if (objPalette && (palette == 0 || objPriority == 0))
					palette = objPalette;

				unsigned color = nesRGB[internalMemory.Read(0x3F00 + (IsRendering() ? palette : 0))];
				RGBA rgbaColor = HexToRGBA(color);

				pixels[scanline * 256 * 4 + x * 4 + 0] = rgbaColor.x;
				pixels[scanline * 256 * 4 + x * 4 + 1] = rgbaColor.y;
				pixels[scanline * 256 * 4 + x * 4 + 2] = rgbaColor.z;
				pixels[scanline * 256 * 4 + x * 4 + 3] = rgbaColor.w;
			}
			// Perform background shifts:
			bgShiftLow  <<= 1;
			bgShiftHigh <<= 1;
            atShiftLow  = (atShiftLow << 1)  | atLatchLow;
            atShiftHigh = (atShiftHigh << 1) | atLatchHigh;
		}


		enum class ScanlinePhase {
			PRE, VISIBLE, POST, NMI
		};

		void Scanline(ScanlinePhase phase)
		{
			static std::uint16_t addr;

			if (phase == ScanlinePhase::NMI && dot == 1) {
				status.vBlank = true;
				if (ctrl.nmiEnable)
					nemu.SetNMI();
			}
			else if (phase == ScanlinePhase::POST && dot == 0) {
				HandleNewFrame(pixels.get());
			}
			else if (phase == ScanlinePhase::VISIBLE || phase == ScanlinePhase::PRE) {

				// Handle Sprites
				if (dot == 1) {
					ClearOAM();
					if (phase == ScanlinePhase::PRE)
						status.spriteOvf = status.spriteHit = false;
				}
				else if (dot == 257) {
					EvalSprites();
				}
				else if (dot == 321) {
					LoadSprites();
				}

				// Handle Background
				if ((dot >= 2 && dot <= 255) || (dot >= 322 && dot <= 337)) {
					Pixel();
					switch (dot % 8) {
					case 1: {
						addr = NametableAddress();
						ShiftBackground();
						break;
					}
					case 2: {
						nt = internalMemory.Read(addr);
						break;
					}
					case 3: {
						addr = AttributeTableAddress();
						break;
					}
					case 4: {
						at = internalMemory.Read(addr);
						if (vAddr.coarseY & 2) at >>= 4;
						if (vAddr.coarseX & 2) at >>= 2;
						break;
					}
					case 5: {
						addr = BackgroundAddress();
						break;
					}
					case 6: {
						bgL = internalMemory.Read(addr);
						break;
					}
					case 7: {
						addr += 8;
						break;
					}
					case 0: {
						bgH = internalMemory.Read(addr);
						ScrollH();
						break;
					}
					}
					return;
				}
				if (dot == 256) {
					Pixel();
					bgH = internalMemory.Read(addr);
					ScrollV();
				}
				else if (dot == 257) {
					Pixel();
					ShiftBackground();
					UpdateScrollH();
				}
				else if (dot >= 280 && dot <= 304) {
					if (phase == ScanlinePhase::PRE) UpdateScrollV();
				}

				else if (dot == 1) {
					addr = NametableAddress();
					if (phase == ScanlinePhase::PRE)
						status.vBlank = false;
				}
				else if (dot >= 321 && dot <= 339) {
					addr = NametableAddress();
				}
				else if (dot == 338) {
					nt = internalMemory.Read(addr);
				}
				else if (dot == 340) {
					nt = internalMemory.Read(addr);
					if (phase == ScanlinePhase::PRE && IsRendering() && frameOdd)
						dot++;
				}
			}
		}

	};

} // namespace ppu
} // namespace nemu
