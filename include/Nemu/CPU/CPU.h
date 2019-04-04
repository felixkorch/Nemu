#pragma once
#include "Nemu/Common.h"
#include "Nemu/Nemu.h"
#include "Nemu/Stack.h"
#include "Nemu/StatusRegister.h"
#include <vector>
#include <iostream>

namespace nemu {
namespace cpu {

    class CPU : public CPUInterface {
		Nemu& nemu;
		std::vector<unsigned> internalRam; // 2KB of Internal RAM
		std::uint8_t   regX;
		std::uint8_t   regY;
		std::uint8_t   regA;
		std::uint16_t  regPC;
		StatusRegister regStatus;
		bool nmi, irq;

		Stack<decltype(internalRam)::iterator> stack;

        constexpr static unsigned Flag_C      = (1 << 0);
        constexpr static unsigned Flag_Z      = (1 << 1);
        constexpr static unsigned Flag_I      = (1 << 2);
        constexpr static unsigned Flag_D      = (1 << 3); // Disabled on the NES (decimal).
        constexpr static unsigned Flag_B      = (1 << 4); // Bits 4 and 5 are used to indicate whether a
        constexpr static unsigned Flag_Unused = (1 << 5); // Software or hardware interrupt occured
        constexpr static unsigned Flag_V      = (1 << 6);
        constexpr static unsigned Flag_N      = (1 << 7);

        constexpr static unsigned ResetVector = 0xFFFC;
        constexpr static unsigned NMIVector   = 0xFFFA;
        constexpr static unsigned IRQVector   = 0xFFFE;

	public:
		CPU(Nemu& nemu) :
			nemu(nemu),
			internalRam(0x800),
			regX(0),
			regY(0),
			regA(0),
			regPC(0),
			regStatus(),
			nmi(false),
			irq(false),
			stack(internalRam.begin() + 0x0100, internalRam.begin() + 0x01FF)
		{}

		virtual void Reset() override
		{
			regPC = Read16(ResetVector);  // Load PC with the reset vector.
		}

		virtual void SetNMI() override
		{
			nmi = true;
		}

		virtual void SetIRQ() override
		{
			irq = true;
		}

		virtual void Execute() override
		{
			if      (nmi) InvokeNMI();
			else if (irq) InvokeIRQ();
			Decode();
		}

	private:

		enum class AddressMode {
			Immediate, Absolute,
			AbsoluteX, AbsoluteY,
			Indirect,  Relative,
			Zeropage,  ZeropageX,
			ZeropageY, IndirectX,
			IndirectY, Implied
		};

		static constexpr unsigned InstructionSizeImmediate = 2;
		static constexpr unsigned InstructionSizeAbsolute  = 3;
		static constexpr unsigned InstructionSizeAbsoluteX = 3;
		static constexpr unsigned InstructionSizeAbsoluteY = 3;
		static constexpr unsigned InstructionSizeIndirect  = 3;
		static constexpr unsigned InstructionSizeRelative  = 2;
		static constexpr unsigned InstructionSizeZeropage  = 2;
		static constexpr unsigned InstructionSizeZeropageX = 2;
		static constexpr unsigned InstructionSizeZeropageY = 2;
		static constexpr unsigned InstructionSizeIndirectX = 2;
		static constexpr unsigned InstructionSizeIndirectY = 2;
		static constexpr unsigned InstructionSizeImplied   = 1;

		unsigned InstructionSize(AddressMode mode)
		{
			switch (mode) {
			case AddressMode::Immediate: return InstructionSizeImmediate;
			case AddressMode::Absolute:  return InstructionSizeAbsolute;
			case AddressMode::AbsoluteX: return InstructionSizeAbsoluteX;
			case AddressMode::AbsoluteY: return InstructionSizeAbsoluteY;
			case AddressMode::Indirect:  return InstructionSizeIndirect;
			case AddressMode::Relative:  return InstructionSizeRelative;
			case AddressMode::Zeropage:  return InstructionSizeZeropage;
			case AddressMode::ZeropageX: return InstructionSizeZeropageX;
			case AddressMode::ZeropageY: return InstructionSizeZeropageY;
			case AddressMode::IndirectX: return InstructionSizeIndirectX;
			case AddressMode::IndirectY: return InstructionSizeIndirectY;
			case AddressMode::Implied:   return InstructionSizeImplied;
			default:                     return InstructionSizeImmediate;
			}
		}

		std::uint16_t GetAddressImmediate()
		{
			return regPC + 1;
		}
		std::uint16_t GetAddressAbsolute()
		{
			std::uint16_t addr = Read16(regPC + 1);
			return addr;
		}
		std::uint16_t GetAddressAbsoluteX()
		{
			std::uint16_t addr = Read16(regPC + 1) + regX;
			return addr;
		}
		std::uint16_t GetAddressAbsoluteY()
		{
			std::uint16_t addr = Read16(regPC + 1) + regY;
			return addr;
		}
		std::uint16_t GetAddressRelative()
		{
			return regPC + 1;
		}
		std::uint16_t GetAddressIndirectX() // Add first then fetch
		{
			std::uint8_t offset = ReadMemory(regPC + 1) + regX;
			std::uint16_t addr = Read16(offset);
			return addr;
		}
		std::uint16_t GetAddressIndirectY() // Fetch first then add
		{
			std::uint8_t offset = ReadMemory(regPC + 1);
			std::uint16_t addr = Read16(offset) + regY;
			return addr;
		}
		std::uint16_t GetAddressZeropage()
		{
			std::uint8_t addr = ReadMemory(regPC + 1);
			return addr;
		}

		std::uint16_t GetAddressZeropageX()
		{
			std::uint8_t addr = ReadMemory(regPC + 1) + regX;
			return addr;
		}
		std::uint16_t GetAddressZeropageY()
		{
			std::uint8_t addr = ReadMemory(regPC + 1) + regY;
			return addr;
		}

		std::uint16_t GetAddress(AddressMode mode)
		{
			switch (mode) {
			case AddressMode::Immediate: return GetAddressImmediate();
			case AddressMode::Absolute:  return GetAddressAbsolute();
			case AddressMode::AbsoluteX: return GetAddressAbsoluteX();
			case AddressMode::AbsoluteY: return GetAddressAbsoluteY();
			case AddressMode::Relative:  return GetAddressRelative();
			case AddressMode::Zeropage:  return GetAddressZeropage();
			case AddressMode::ZeropageX: return GetAddressZeropageX();
			case AddressMode::ZeropageY: return GetAddressZeropageY();
			case AddressMode::IndirectX: return GetAddressIndirectX();
			case AddressMode::IndirectY: return GetAddressIndirectY();
			default:                     return GetAddressImmediate();
			}
		}

		// Write 256 successive bytes to the OAMDMA register.
		void DmaOam(unsigned bank)
		{
			for (int i = 0; i < 256; i++)
				WriteMemory(0x2014, ReadMemory(bank * 0x100 + i));
		}

		void WriteMemory(std::size_t index, unsigned value)
		{
            // Internal RAM
			if (index <= 0x1FFF) {
				internalRam[index] = value;
				return; 
			}
			// PPU Access
			if (index <= 0x3FFF) {
				nemu.PPUWrite(index % 8, value);
				return;
			}

			// Joypad Write strobe
			if (index == 0x4016) {
				nemu.JoypadWrite(value & 1);
				return;
			}

			// DMA-OAM Access
			if (index == 0x4014) {
				DmaOam(value);
				return;
			}

			// Cartridge Access
			if (index >= 0x4020 && index <= 0xFFFF) {
				nemu.CartridgeWritePRG(index, value);
				return;
			}
		}

        unsigned ReadMemory(std::size_t index)
		{
			// Internal RAM
			if (index <= 0x1FFF) {
				return internalRam[index];
			}
			// PPU Access
			if (index <= 0x3FFF) {
				return nemu.PPURead(index % 8);
			}

			// Joypad Read (0)
			if (index == 0x4016) {
				return nemu.JoypadRead(0);
			}

			// Cartridge Access
			if (index >= 0x4020 && index <= 0xFFFF) {
				return nemu.CartridgeReadPRG(index);
			}
			return 0; // Default
		}


		unsigned Read16(std::size_t index)
		{
			return ReadMemory(index) | (ReadMemory(index + 1) << 8);
		}

		void InvokeNMI()
		{
			stack.Push((regPC >> 8) & 0xFF);
			stack.Push(regPC & 0xFF);
			stack.Push((regStatus & ~Flag_B) | Flag_Unused);
			regStatus.I = 1;
			regPC = Read16(NMIVector);
			nmi = false;
		}

		void InvokeIRQ()
		{
			if (!regStatus.I) {
				stack.Push((regPC >> 8) & 0xFF);
				stack.Push(regPC & 0xFF);
				stack.Push((regStatus & ~Flag_B) | Flag_Unused);
				regStatus.I = 1;
				regPC = Read16(IRQVector);
				irq = false;
			}
		}

		void Decode() // Fetches & decodes an instruction
		{
			switch (ReadMemory(regPC)) {
			case 0x00: OpBRK();                             break;
			case 0xA0: OpLD(AddressMode::Immediate, regY);  break;
			case 0xA4: OpLD(AddressMode::Zeropage, regY);   break;
			case 0xB4: OpLD(AddressMode::ZeropageX, regY);  break;
			case 0xAC: OpLD(AddressMode::Absolute, regY);   break;
			case 0xBC: OpLD(AddressMode::AbsoluteX, regY);  break;
			case 0xA2: OpLD(AddressMode::Immediate, regX);  break;
			case 0xA6: OpLD(AddressMode::Zeropage, regX);   break;
			case 0xB6: OpLD(AddressMode::ZeropageY, regX);  break;
			case 0xAE: OpLD(AddressMode::Absolute, regX);   break;
			case 0xBE: OpLD(AddressMode::AbsoluteY, regX);  break;
			case 0xEA: OpNOP();                             break;
			case 0x18: OpCLR(Flag_C);                       break;
			case 0xD8: OpCLR(Flag_D);                       break;
			case 0x58: OpCLR(Flag_I);                       break;
			case 0xB8: OpCLR(Flag_V);                       break;
			case 0x38: OpSET(Flag_C);                       break;
			case 0xF8: OpSET(Flag_D);                       break;
			case 0x78: OpSET(Flag_I);                       break;
			case 0xA9: OpLD(AddressMode::Immediate, regA);  break;
			case 0xA5: OpLD(AddressMode::Zeropage, regA);   break;
			case 0xB5: OpLD(AddressMode::ZeropageX, regA);  break;
			case 0xAD: OpLD(AddressMode::Absolute, regA);   break;
			case 0xBD: OpLD(AddressMode::AbsoluteX, regA);  break;
			case 0xB9: OpLD(AddressMode::AbsoluteY, regA);  break;
			case 0xA1: OpLD(AddressMode::IndirectX, regA);  break;
			case 0xB1: OpLD(AddressMode::IndirectY, regA);  break;
			case 0x69: OpADC(AddressMode::Immediate);       break;
			case 0x65: OpADC(AddressMode::Zeropage);        break;
			case 0x75: OpADC(AddressMode::ZeropageX);       break;
			case 0x6D: OpADC(AddressMode::Absolute);        break;
			case 0x7D: OpADC(AddressMode::AbsoluteX);       break;
			case 0x79: OpADC(AddressMode::AbsoluteY);       break;
			case 0x61: OpADC(AddressMode::IndirectX);       break;
			case 0x71: OpADC(AddressMode::IndirectY);       break;
			case 0xE9: OpSBC(AddressMode::Immediate);       break;
			case 0xE5: OpSBC(AddressMode::Zeropage);        break;
			case 0xF5: OpSBC(AddressMode::ZeropageX);       break;
			case 0xED: OpSBC(AddressMode::Absolute);        break;
			case 0xFD: OpSBC(AddressMode::AbsoluteX);       break;
			case 0xF9: OpSBC(AddressMode::AbsoluteY);       break;
			case 0xE1: OpSBC(AddressMode::IndirectX);       break;
			case 0xF1: OpSBC(AddressMode::IndirectY);       break;
			case 0x85: OpST(AddressMode::Zeropage, regA);   break;
			case 0x95: OpST(AddressMode::ZeropageX, regA);  break;
			case 0x8D: OpST(AddressMode::Absolute, regA);   break;
			case 0x9D: OpST(AddressMode::AbsoluteX, regA);  break;
			case 0x99: OpST(AddressMode::AbsoluteY, regA);  break;
			case 0x81: OpST(AddressMode::IndirectX, regA);  break;
			case 0x91: OpST(AddressMode::IndirectY, regA);  break;
			case 0x86: OpST(AddressMode::Zeropage, regX);   break;
			case 0x96: OpST(AddressMode::ZeropageY, regX);  break;
			case 0x8E: OpST(AddressMode::Absolute, regX);   break;
			case 0x84: OpST(AddressMode::Zeropage, regY);   break;
			case 0x94: OpST(AddressMode::ZeropageX, regY);  break;
			case 0x8C: OpST(AddressMode::Absolute, regY);   break;
			case 0x4C: OpJMPAbsolute();                     break;
			case 0x6C: OpJMPIndirect();                     break;
			case 0x20: OpJSR();                             break;
			case 0x48: OpPHA();                             break;
			case 0x08: OpPHP();                             break;
			case 0x68: OpPLA();                             break;
			case 0x28: OpPLP();                             break;
			case 0x40: OpRTI();                             break;
			case 0x60: OpRTS();                             break;
			case 0x10: OpBRA(!regStatus.N);                 break;
			case 0xF0: OpBRA(regStatus.Z);                  break;
			case 0x90: OpBRA(!regStatus.C);                 break;
			case 0xB0: OpBRA(regStatus.C);                  break;
			case 0x30: OpBRA(regStatus.N);                  break;
			case 0xD0: OpBRA(!regStatus.Z);                 break;
			case 0x50: OpBRA(!regStatus.V);                 break;
			case 0x70: OpBRA(regStatus.V);                  break;
			case 0x24: OpBIT(AddressMode::Zeropage);        break;
			case 0x2C: OpBIT(AddressMode::Absolute);        break;
			case 0x88: OpDEY();                             break;
			case 0xCA: OpDEX();                             break;
			case 0xC6: OpDEC(AddressMode::Zeropage);        break;
			case 0xD6: OpDEC(AddressMode::ZeropageX);       break;
			case 0xCE: OpDEC(AddressMode::Absolute);        break;
			case 0xDE: OpDEC(AddressMode::AbsoluteX);       break;
			case 0x8A: OpTXA();                             break;
			case 0xAA: OpTAX();                             break;
			case 0xA8: OpTAY();                             break;
			case 0xBA: OpTSX();                             break;
			case 0x9A: OpTXS();                             break;
			case 0x98: OpTYA();                             break;
			case 0x0A: OpASLImplied();                      break;
			case 0x06: OpASL(AddressMode::Zeropage);        break;
			case 0x16: OpASL(AddressMode::ZeropageX);       break;
			case 0x0E: OpASL(AddressMode::Absolute);        break;
			case 0x1E: OpASL(AddressMode::AbsoluteX);       break;
			case 0x4A: OpLSRImplied();                      break;
			case 0x46: OpLSR(AddressMode::Zeropage);        break;
			case 0x56: OpLSR(AddressMode::ZeropageX);       break;
			case 0x4E: OpLSR(AddressMode::Absolute);        break;
			case 0x5E: OpLSR(AddressMode::AbsoluteX);       break;
			case 0x2A: OpROLImplied();                      break;
			case 0x26: OpROL(AddressMode::Zeropage);        break;
			case 0x36: OpROL(AddressMode::ZeropageX);       break;
			case 0x2E: OpROL(AddressMode::Absolute);        break;
			case 0x3E: OpROL(AddressMode::AbsoluteX);       break;
			case 0x6A: OpRORImplied();                      break;
			case 0x66: OpROR(AddressMode::Zeropage);        break;
			case 0x76: OpROR(AddressMode::ZeropageX);       break;
			case 0x6E: OpROR(AddressMode::Absolute);        break;
			case 0x7E: OpROR(AddressMode::AbsoluteX);       break;
			case 0x29: OpAND(AddressMode::Immediate);       break;
			case 0x25: OpAND(AddressMode::Zeropage);        break;
			case 0x35: OpAND(AddressMode::ZeropageX);       break;
			case 0x2D: OpAND(AddressMode::Absolute);        break;
			case 0x3D: OpAND(AddressMode::AbsoluteX);       break;
			case 0x39: OpAND(AddressMode::AbsoluteY);       break;
			case 0x21: OpAND(AddressMode::IndirectX);       break;
			case 0x31: OpAND(AddressMode::IndirectY);       break;
			case 0xC9: OpCMP(AddressMode::Immediate, regA); break;
			case 0xC5: OpCMP(AddressMode::Zeropage, regA);  break;
			case 0xD5: OpCMP(AddressMode::ZeropageX, regA); break;
			case 0xCD: OpCMP(AddressMode::Absolute, regA);  break;
			case 0xDD: OpCMP(AddressMode::AbsoluteX, regA); break;
			case 0xD9: OpCMP(AddressMode::AbsoluteY, regA); break;
			case 0xC1: OpCMP(AddressMode::IndirectX, regA); break;
			case 0xD1: OpCMP(AddressMode::IndirectY, regA); break;
			case 0xE0: OpCMP(AddressMode::Immediate, regX); break;
			case 0xE4: OpCMP(AddressMode::Zeropage, regX);  break;
			case 0xEC: OpCMP(AddressMode::Absolute, regX);  break;
			case 0xC0: OpCMP(AddressMode::Immediate, regY); break;
			case 0xC4: OpCMP(AddressMode::Zeropage, regY);  break;
			case 0xCC: OpCMP(AddressMode::Absolute, regY);  break;
			case 0x09: OpORA(AddressMode::Immediate);       break;
			case 0x05: OpORA(AddressMode::Zeropage);        break;
			case 0x15: OpORA(AddressMode::ZeropageX);       break;
			case 0x0D: OpORA(AddressMode::Absolute);        break;
			case 0x1D: OpORA(AddressMode::AbsoluteX);       break;
			case 0x19: OpORA(AddressMode::AbsoluteY);       break;
			case 0x01: OpORA(AddressMode::IndirectX);       break;
			case 0x11: OpORA(AddressMode::IndirectY);       break;
			case 0x49: OpEOR(AddressMode::Immediate);       break;
			case 0x45: OpEOR(AddressMode::Zeropage);        break;
			case 0x55: OpEOR(AddressMode::ZeropageX);       break;
			case 0x4D: OpEOR(AddressMode::Absolute);        break;
			case 0x5D: OpEOR(AddressMode::AbsoluteX);       break;
			case 0x59: OpEOR(AddressMode::AbsoluteY);       break;
			case 0x41: OpEOR(AddressMode::IndirectX);       break;
			case 0x51: OpEOR(AddressMode::IndirectY);       break;
			case 0xE6: OpINC(AddressMode::Zeropage);        break;
			case 0xF6: OpINC(AddressMode::ZeropageX);       break;
			case 0xEE: OpINC(AddressMode::Absolute);        break;
			case 0xFE: OpINC(AddressMode::AbsoluteX);       break;
			case 0xE8: OpINX();                             break;
			case 0xC8: OpINY();                             break;
			default: // Illegal opcode
				std::cout << "Error: Illegal op-code" << std::endl;
				std::cin.get();
				break;
			};
		}

		/// Overflow:
		///
		/// The result of a signed addition or subtraction doesn't fit into a signed byte.
		/// For addition this means that bit 7 is set; the operation
		/// overflowed into the sign bit. For subtraction this means
		/// that bit 7 is not set; a carry from the 6th place shifted
		/// the sign bit out of its place. Note that overflow can't
		/// occur if the operands have different signs, since it will
		/// always be less than the positive one.

		
		void OpADC(AddressMode mode)
		{
			auto oper = ReadMemory(GetAddress(mode));
            unsigned result = regA + oper + regStatus.C;
			bool overflow = !((regA ^ oper) & Bit7) && ((regA ^ result) & Bit7);
            regStatus.Z = (result & 0xFF) == 0;
            regStatus.C = result > 0xFF;
            regStatus.V = overflow;
            regStatus.N = result & Bit7;
			regA = result & 0xFF;
			regPC += InstructionSize(mode);
		}

		
		void OpSBC(AddressMode mode)
		{
			auto oper = ReadMemory(GetAddress(mode));
            unsigned result = regA - oper - !regStatus.C;
			bool overflow = ((regA ^ result) & Bit7) && ((regA ^ oper) & Bit7);
            regStatus.Z = (result & 0xFF) == 0;
            regStatus.C = result < Bit8;
            regStatus.V = overflow;
            regStatus.N = result & Bit7;
			regA = result & 0xFF;
			regPC += InstructionSize(mode);
		}
		
		void OpCMP(AddressMode mode, std::uint8_t& reg)
		{
			auto oper = ReadMemory(GetAddress(mode));
			unsigned result = reg - oper;
            regStatus.Z = reg == oper;
            regStatus.N = result & Bit7;
            regStatus.C = result < Bit8;
			regPC += InstructionSize(mode);
		}
		
		void OpAND(AddressMode mode)
		{
			auto oper = ReadMemory(GetAddress(mode));
			regA &= oper;
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC += InstructionSize(mode);
		}
		
		void OpEOR(AddressMode mode)
		{
			auto oper = ReadMemory(GetAddress(mode));
			regA ^= oper;
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC += InstructionSize(mode);
		}
		
		/// Read -> Modify -> Write

		// Rotate Left
		void OpROL(AddressMode mode)
		{
			std::uint16_t address = GetAddress(mode);
			unsigned oper = ReadMemory(address);
			oper <<= 1;                                        // Holds carry in bit 8, Shifts left one bit
			oper = regStatus.C ? oper | Bit0 : oper & ~Bit0;   // Changes bit 0 to whatever carry is
            regStatus.C = oper & Bit8;                         // Sets carry flag to whatever bit 8 is
			WriteMemory(address, oper & 0xFF);
			SetFlagNegative(oper);
			SetFlagZero(oper);
			regPC += InstructionSize(mode);
		}

		void OpROLImplied()
		{
			unsigned temp = regA;
			temp <<= 1;
            temp = regStatus.C ? temp | Bit0 : temp & ~Bit0;
            regStatus.C = temp & Bit8;
			regA = temp & 0xFF;
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC += InstructionSize(AddressMode::Implied);
		}
		
		// Rotate Right
		void OpROR(AddressMode mode)
		{
			std::uint16_t address = GetAddress(mode);
			unsigned oper = ReadMemory(address);
			oper = regStatus.C ? oper | Bit8 : oper & ~Bit8;
            regStatus.C = oper & Bit0;
			oper >>= 1;
			WriteMemory(address, oper & 0xFF);
			SetFlagNegative(oper);
			SetFlagZero(oper);
			regPC += InstructionSize(mode);
		}

		void OpRORImplied()
		{
			unsigned temp = regA;
            temp = regStatus.C ? temp | Bit8 : temp & ~Bit8;
            regStatus.C = temp & Bit0;
			temp >>= 1;
			regA = temp & 0xFF;
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC += InstructionSize(AddressMode::Implied);
		}
		
		// Arithmetic shift left
		void OpASL(AddressMode mode)
		{
			std::uint16_t address = GetAddress(mode);
			std::uint8_t oper = ReadMemory(address);
            regStatus.C = oper & Bit7;
			oper <<= 1;
			WriteMemory(address, oper);
			SetFlagNegative(oper);
			SetFlagZero(oper);
			regPC += InstructionSize(mode);
		}

		void OpASLImplied() // Special case for Accumulator ASL
        {
            regStatus.C = regA & Bit7;
			regA <<= 1;
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC += InstructionSize(AddressMode::Implied);
		}
		
		void OpLSR(AddressMode mode)
		{
			std::uint16_t address = GetAddress(mode);
			std::uint8_t oper = ReadMemory(address);
            regStatus.C = oper & Bit0;
			oper >>= 1;
			WriteMemory(address, oper);
            regStatus.Z = oper == 0;
            regStatus.N = 0;
			regPC += InstructionSize(mode);
		}

		void OpLSRImplied() // Special case for Accumulator LSR
        {
            regStatus.C = regA & Bit0;
			regA >>= 1;
            regStatus.Z = regA == 0;
            regStatus.N = 0;
			regPC += InstructionSize(AddressMode::Implied);
		}

		/// Increase / Decrease Registers 
		
		void OpDEC(AddressMode mode)
		{
			std::uint16_t address = GetAddress(mode);
			std::uint8_t oper = ReadMemory(address);
			oper--;
			WriteMemory(address, oper);
            regStatus.Z = oper == 0;
            regStatus.N = oper & Bit7;
			regPC += InstructionSize(mode);
		}

		void OpINC(AddressMode mode)
		{
			std::uint16_t address = GetAddress(mode);
			std::uint8_t oper = ReadMemory(address);
			oper++;
			WriteMemory(address, oper);
			SetFlagNegative(oper);
			SetFlagZero(oper);
			regPC += InstructionSize(mode);
		}

		void OpDEX()
		{
			regX--;
			SetFlagNegative(regX);
			SetFlagZero(regX);
			regPC++;
		}
		void OpDEY()
		{
			regY--;
            regStatus.Z = regY == 0;
            regStatus.N = regY & Bit7;
			regPC++;
		}

		void OpINX()
		{
			regX++;
			SetFlagNegative(regX);
			SetFlagZero(regX);
			regPC++;
		}
		void OpINY()
		{
			regY++;
			SetFlagNegative(regY);
			SetFlagZero(regY);
			regPC++;
		}

		/// Flow control
		void OpJMPAbsolute()
		{
			regPC = Read16(regPC + 1);
		}

		void OpJMPIndirect()
		{
			std::uint16_t offset = Read16(regPC + 1);
			regPC = Read16(offset);
		}
		void OpJSR()
		{
			std::uint16_t addr = Read16(regPC + 1);
			regPC += 2;
			stack.Push((regPC >> 8) & 0xFF); // Push PC_High
			stack.Push(regPC & 0xFF);        // Push PC_Low
			regPC = addr;
		}
		
		void OpBRA(bool condition)
		{
			std::int8_t oper = ReadMemory(GetAddress(AddressMode::Immediate));
			regPC += condition ? oper + InstructionSize(AddressMode::Immediate) : InstructionSize(AddressMode::Immediate);
		}

		void OpTXA()
		{
			regA = regX;
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC++;
		}
		void OpTAX()
		{
			regX = regA;
			SetFlagNegative(regX);
			SetFlagZero(regX);
			regPC++;
		}
		void OpTAY()
		{
			regY = regA;
			SetFlagNegative(regY);
			SetFlagZero(regY);
			regPC++;
		}
		void OpTSX()
		{
			regX = stack.GetSP();
			SetFlagNegative(regX);
			SetFlagZero(regX);
			regPC++;
		}
		void OpTXS()
		{
			stack.SetSP(regX);
			regPC++;
		}
		void OpTYA()
		{
			regA = regY;
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC++;
		}

		void OpLD(AddressMode mode, std::uint8_t& reg)
		{
			reg = ReadMemory(GetAddress(mode));
			SetFlagNegative(reg);
			SetFlagZero(reg);
			regPC += InstructionSize(mode);
		}

		void OpST(AddressMode mode, std::uint8_t& reg)
		{
			WriteMemory(GetAddress(mode), reg);
			regPC += InstructionSize(mode);
		}


		/// Bit Operations
		void OpORA(AddressMode mode)
		{
			regA |= ReadMemory(GetAddress(mode));
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC += InstructionSize(mode);
		}
		
		void OpBIT(AddressMode mode)
		{
			auto oper = ReadMemory(GetAddress(mode));
            regStatus.N = oper & Bit7;
            regStatus.V = oper & Bit6;
            regStatus.Z = (oper & regA) == 0;
			regPC += InstructionSize(mode);
		}
        void OpCLR(unsigned flag)
		{
            regStatus = (unsigned)regStatus & ~flag;
			regPC++;
		}
        void OpSET(unsigned flag)
		{
            regStatus = (unsigned)regStatus | flag;
			regPC++;
		}

		// Stack operations
		void OpPHA()
		{
			stack.Push(regA);
			regPC++;
		}

		void OpPHP()
		{
			stack.Push(regStatus | Flag_B | Flag_Unused);
			regPC++;
		}
		void OpPLA()
		{
			regA = stack.Pop();
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC++;
		}
		void OpPLP()
		{
			regStatus = stack.Pop();
			regPC++;
		}
		void OpRTI()
		{
			regStatus = stack.Pop();
			unsigned PC_low = stack.Pop();
			unsigned PC_high = stack.Pop();
			regPC = (PC_high << 8) | PC_low;
		}
		void OpRTS()
		{
			unsigned PC_low = stack.Pop();
			unsigned PC_high = stack.Pop();
			regPC = ((PC_high << 8) | PC_low) + 1;
		}

		void OpNOP()
		{
			regPC++;
		}
		
		// Software Interrupt
		void OpBRK()
		{
			regPC += 2;
			stack.Push((regPC >> 8) & 0xFF);
			stack.Push(regPC & 0xFF);
			stack.Push(regStatus | Flag_B | Flag_Unused);
            regStatus.I = 1;
			regPC = Read16(IRQVector);
		}

		void SetFlagNegative(unsigned oper)
		{
            regStatus.N = oper & Bit7; // If 7th bit is 1, set negative
		}

		void SetFlagZero(unsigned oper)
        {
            regStatus.Z = oper == 0;   // If register is 0, set zero
		}

	};

} // namespace cpu
} // namespace nemu
