#pragma once
#include "Nemu/Stack.h"
#include "Nemu/NESMemory.h"
#include "Nemu/Mapper/NROM256Mapper.h"
#include "Nemu/StatusRegister.h"
#include <vector>
#include <array>
#include <iostream>

namespace nemu
{
	template <class T>
	class VectorMemory : public std::vector<T> { // Temporary class to use standard vector as memory
	public:

		VectorMemory()
			: std::vector<T>(0xFFFF) {}

		VectorMemory(const std::vector<T> vec)
			: std::vector<T>(vec) {}

		std::uint16_t Get16At(std::size_t offset)
		{
			return *((std::uint16_t*)(this->data() + offset));
		}
	};

	template <class Memory>
	class CPU {
	private:
		using uint   = std::uint32_t;
		using int32  = std::int32_t;
		using uint32 = std::uint32_t;
		using uint16 = std::uint16_t;
		using uint8  = std::uint8_t;
		using int8   = std::int8_t;

		uint8  regX;
		uint8  regY;
		uint8  regA;
		uint16 regPC;
        StatusRegister regStatus;

		Memory& memory;
		Stack<typename Memory::iterator> stack;

        constexpr static uint Flag_C      = (1 << 0);
        constexpr static uint Flag_Z      = (1 << 1);
        constexpr static uint Flag_I      = (1 << 2);
        constexpr static uint Flag_D      = (1 << 3); // Disabled on the NES (decimal).
        constexpr static uint Flag_B      = (1 << 4); // Bits 4 and 5 are used to indicate whether a
        constexpr static uint Flag_Unused = (1 << 5); // Software or hardware interrupt occured
        constexpr static uint Flag_V      = (1 << 6);
        constexpr static uint Flag_N      = (1 << 7);

        constexpr static uint Bit0 = (1 << 0);
        constexpr static uint Bit1 = (1 << 1);
        constexpr static uint Bit2 = (1 << 2);
        constexpr static uint Bit3 = (1 << 3);
        constexpr static uint Bit4 = (1 << 4);
        constexpr static uint Bit5 = (1 << 5);
        constexpr static uint Bit6 = (1 << 6);
        constexpr static uint Bit7 = (1 << 7);
        constexpr static uint Bit8 = (1 << 8);

        constexpr static uint ResetVector = 0xFFFC;
        constexpr static uint NMIVector   = 0xFFFA;
        constexpr static uint IRQVector   = 0xFFFE;

	public:
		CPU(Memory& mem)
			: regX(0),
			  regY(0),
			  regA(0),
			  regPC(0),
			  regStatus(),
			  stack(mem.begin() + 0x0100, mem.begin() + 0x01FF), // Stack range 0x0100 -> 0x01FF.
			  memory(mem)
		{
			Reset();
		}

		/// -------------------------------------------PUBLIC FUNCTIONS------------------------------------------------------- ///

		void Reset()
		{
			regPC = memory.Get16At(ResetVector);  // Load PC with the reset vector.
		}

		void InvokeNMI()
		{
			stack.Push((regPC >> 8) & 0xFF);
			stack.Push(regPC & 0xFF);
            stack.Push((regStatus & ~Flag_B) | Flag_Unused);
            regStatus.I = 1;
			regPC = memory.Get16At(NMIVector);
		}

		void InvokeIRQ()
		{
            if (!regStatus.I) {
				stack.Push((regPC >> 8) & 0xFF);
				stack.Push(regPC & 0xFF);
                stack.Push((regStatus & ~Flag_B) | Flag_Unused);
                regStatus.I = 1;
				regPC = memory.Get16At(IRQVector);
			}
		}

		void Execute()
		{
			Decode();
		}

		void PrintFlags()
		{
            std::cout << "[C: " << +regStatus.C      << " | "
                      << "Z:  " << +regStatus.Z      << " | "
                      << "I:  " << +regStatus.I      << " | "
                      << "B:  " << +regStatus.B      << " | "
                      << "U:  " << +regStatus.Unused << " | "
                      << "D:  " << +regStatus.D      << " | "
                      << "N:  " << +regStatus.N      << " | "
                      << "V:  " << +regStatus.V      << " ] " << std::endl;
		}

		void PrintRegisters()
		{
			std::cout << "A: " << +regA << std::endl;
			std::cout << "X: " << +regX << std::endl;
			std::cout << "Y: " << +regY << std::endl;
		}

	/// -------------------------------------------PRIVATE FUNCTIONS------------------------------------------------------- ///

	private:

		enum class AddressMode {
			Immediate, Absolute,
			AbsoluteX, AbsoluteY,
			Indirect,  Relative,
			Zeropage,  ZeropageX,
			ZeropageY, IndirectX,
			IndirectY, Implied
		};

		static constexpr int InstructionSizeImmediate = 2;
		static constexpr int InstructionSizeAbsolute  = 3;
		static constexpr int InstructionSizeAbsoluteX = 3;
		static constexpr int InstructionSizeAbsoluteY = 3;
		static constexpr int InstructionSizeIndirect  = 3;
		static constexpr int InstructionSizeRelative  = 2;
		static constexpr int InstructionSizeZeropage  = 2;
		static constexpr int InstructionSizeZeropageX = 2;
		static constexpr int InstructionSizeZeropageY = 2;
		static constexpr int InstructionSizeIndirectX = 2;
		static constexpr int InstructionSizeIndirectY = 2;
		static constexpr int InstructionSizeImplied   = 1;

		int InstructionSize(AddressMode mode)
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
			}
		}

		uint8& GetOperandImmediate()
		{
			return memory[regPC + 1];
		}

		uint8& GetOperandAbsolute()
		{
			uint16 addr = memory.Get16At(regPC + 1);
			return memory[addr];
		}
		uint8& GetOperandAbsoluteX()
		{
			uint16 addr = memory.Get16At(regPC + 1) + regX;
			return memory[addr];
		}
		uint8& GetOperandAbsoluteY()
		{
			uint16 addr = memory.Get16At(regPC + 1) + regY;
			return memory[addr];
		}
		uint8& GetOperandRelative()
		{
			return memory[regPC + 1];
		}
		uint8& GetOperandIndirectX() // Add first then fetch
		{
			uint8 offset = memory[regPC + 1] + regX;
			uint16 addr = memory.Get16At(offset);
			return memory[addr];
		}
		uint8& GetOperandIndirectY() // Fetch first then add
		{
			uint8 offset = memory[regPC + 1];
			uint16 addr = memory.Get16At(offset) + regY;
			return memory[addr];
		}
		uint8& GetOperandZeropage()
		{
			uint8 offset = memory[regPC + 1];
			return memory[offset];
		}

		uint8& GetOperandZeropageX()
		{
			uint8 offset = memory[regPC + 1] + regX;
			return memory[offset];
		}
		uint8& GetOperandZeropageY()
		{
			uint8 offset = memory[regPC + 1] + regY;
			return memory[offset];
		}

		uint8& GetOperand(AddressMode mode)
		{
			switch (mode) {
			case AddressMode::Immediate: return GetOperandImmediate();
			case AddressMode::Absolute:  return GetOperandAbsolute();
			case AddressMode::AbsoluteX: return GetOperandAbsoluteX();
			case AddressMode::AbsoluteY: return GetOperandAbsoluteY();
			case AddressMode::Relative:  return GetOperandRelative();
			case AddressMode::Zeropage:  return GetOperandZeropage();
			case AddressMode::ZeropageX: return GetOperandZeropageX();
			case AddressMode::ZeropageY: return GetOperandZeropageY();
			case AddressMode::IndirectX: return GetOperandIndirectX();
			case AddressMode::IndirectY: return GetOperandIndirectY();
			}
		}

		void Decode() // Fetches & decodes an instruction
		{
			//std::cout << "0x" << std::hex << regPC << std::endl;
			switch (memory[regPC]) {
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
			case 0x69: OpAD(AddressMode::Immediate);        break;
			case 0x65: OpAD(AddressMode::Zeropage);         break;
			case 0x75: OpAD(AddressMode::ZeropageX);        break;
			case 0x6D: OpAD(AddressMode::Absolute);         break;
			case 0x7D: OpAD(AddressMode::AbsoluteX);        break;
			case 0x79: OpAD(AddressMode::AbsoluteY);        break;
			case 0x61: OpAD(AddressMode::IndirectX);        break;
			case 0x71: OpAD(AddressMode::IndirectY);        break;
			case 0xE9: OpSB(AddressMode::Immediate);        break;
			case 0xE5: OpSB(AddressMode::Zeropage);         break;
			case 0xF5: OpSB(AddressMode::ZeropageX);        break;
			case 0xED: OpSB(AddressMode::Absolute);         break;
			case 0xFD: OpSB(AddressMode::AbsoluteX);        break;
			case 0xF9: OpSB(AddressMode::AbsoluteY);        break;
			case 0xE1: OpSB(AddressMode::IndirectX);        break;
			case 0xF1: OpSB(AddressMode::IndirectY);        break;
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
				break;
			};
		}

		/// The definition of "Overflow" on the 6502 is that
		/// the result of a signed addition or subtraction doesn't fit
		/// into a signed byte.
		///
		/// For addition this means that bit 7 is set; the operation
		/// overflowed into the sign bit. For subtraction this means
		/// that bit 7 is not set; a carry from the 6th place shifted
		/// the sign bit out of its place. Note that overflow can't
		/// occur if the operands have different signs, since it will
		/// always be less than the positive one.

		
		void OpADC(AddressMode mode)
		{
			uint8& oper = GetOperand(mode);
            uint result = regA + oper + regStatus.C;
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
			uint8& oper = GetOperand(mode);
            uint result = regA - oper - !regStatus.C;
			bool overflow = ((regA ^ result) & Bit7) && ((regA ^ oper) & Bit7);
            regStatus.Z = (result & 0xFF) == 0;
            regStatus.C = result < Bit8;
            regStatus.V = overflow;
            regStatus.N = result & Bit7;
			regA = result & 0xFF;
			regPC += InstructionSize(mode);
		}
		
		void OpCMP(AddressMode mode, uint8& reg)
		{
			uint8& oper = GetOperand(mode);
			uint result = reg - oper;
            regStatus.Z = reg == oper;
            regStatus.N = result & Bit7;
            regStatus.C = result < Bit8;
			regPC += InstructionSize(mode);
		}
		
		void OpAND(AddressMode mode)
		{
			uint8& oper = GetOperand(mode);
			regA &= oper;
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC += InstructionSize(mode);
		}
		
		void OpEOR(AddressMode mode)
		{
			uint8& oper = GetOperand(mode);
			regA ^= oper;
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC += InstructionSize(mode);
		}
		
		void OpROL(AddressMode mode)
		{
			uint8& oper = GetOperand(mode);
            uint temp = oper;                                  // Holds carry in bit 8
            temp <<= 1;                                        // Shifts left one bit
            temp = regStatus.C ? temp | Bit0 : temp & ~Bit0;   // Changes bit 0 to whatever carry is
            regStatus.C = temp & Bit8;                         // Sets carry flag to whatever bit 8 is
			oper = temp & 0xFF;
			SetFlagNegative(oper);
			SetFlagZero(oper);
			regPC += InstructionSize(mode);
		}

		void OpROLImplied() // Special case for Accumulator ROL
		{
			uint temp = regA;
			temp <<= 1;
            temp = regStatus.C ? temp | Bit0 : temp & ~Bit0;
            regStatus.C = temp & Bit8;
			regA = temp & 0xFF;
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC += InstructionSize(AddressMode::Implied);
		}
		
		void OpROR(AddressMode mode)
		{
			uint8& oper = GetOperand(mode);
			uint temp = oper;
            temp = regStatus.C ? temp | Bit8 : temp & ~Bit8;
            regStatus.C = temp & Bit0;
			temp >>= 1;
			oper = temp & 0xFF;
			SetFlagNegative(oper);
			SetFlagZero(oper);
			regPC += InstructionSize(mode);
		}

		void OpRORImplied() // Special case for Accumulator ROR
		{
			uint temp = regA;
            temp = regStatus.C ? temp | Bit8 : temp & ~Bit8;
            regStatus.C = temp & Bit0;
			temp >>= 1;
			regA = temp & 0xFF;
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC += InstructionSize(AddressMode::Implied);
		}
		
		void OpASL(AddressMode mode)
		{
			uint8& oper = GetOperand(mode);
            regStatus.C = oper & Bit7;
			oper <<= 1;
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
			uint8& oper = GetOperand(mode);
            regStatus.C = oper & Bit0;
			oper >>= 1;
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

		/* Decrease Operations */
		
		void OpDEC(AddressMode mode)
		{
			uint8& oper = GetOperand(mode);
			oper--;
            regStatus.Z = oper == 0;
            regStatus.N = oper & Bit7;
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

		/* Increase Operations */
		
		void OpINC(AddressMode mode) // Increase operations
		{
			uint8& oper = GetOperand(mode);
			oper++;
			SetFlagNegative(oper);
			SetFlagZero(oper);
			regPC += InstructionSize(mode);
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

		/* Transfer Operations */
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

		void OpLD(AddressMode mode, uint8& reg) // Load operations
		{
			uint8& oper = GetOperand(mode);
			reg = oper;
			SetFlagNegative(reg);
			SetFlagZero(reg);
			regPC += InstructionSize(mode);
		}
		
		void OpST(AddressMode mode, uint8& reg) // Store operations
		{
			uint8& oper = GetOperand(mode);
			oper = reg;
			regPC += InstructionSize(mode);
		}
		
		void OpORA(AddressMode mode)
		{
			uint8& oper = GetOperand(mode);
			regA |= oper;
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC += InstructionSize(mode);
		}
		
		void OpBIT(AddressMode mode)
		{
			uint8& oper = GetOperand(mode);
            regStatus.N = oper & Bit7;
            regStatus.V = oper & Bit6;
            regStatus.Z = (oper & regA) == 0;
			regPC += InstructionSize(mode);
		}
        void OpCLR(int flag)
		{
            regStatus = (int)regStatus & ~flag;
			regPC++;
		}
        void OpSET(int flag)
		{
            regStatus = (int)regStatus | flag;
			regPC++;
		}

		/* Jump operations */

		void OpJMPAbsolute()
		{
			regPC = memory.Get16At(regPC + 1);
		}

		void OpJMPIndirect()
		{
			uint16 offset = memory.Get16At(regPC + 1);
			regPC = memory.Get16At(offset);
		}
		void OpJSR()
		{
			uint16 addr = memory.Get16At(regPC + 1);
			regPC += 2;
			stack.Push((regPC >> 8) & 0xFF); // Push PC_High
			stack.Push(regPC & 0xFF);        // Push PC_Low
			regPC = addr;
		}
		void OpBRA(AddressMode mode, bool condition) // Branch operations
		{
			int8 oper = GetOperand(mode);
			regPC += condition ? oper + InstructionSize(AddressMode::Immediate) : InstructionSize(AddressMode::Immediate);
		}

		/* Stack operations */
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
			uint16 PC_low = stack.Pop();
			uint16 PC_high = stack.Pop();
			regPC = (PC_high << 8) | PC_low;
		}
		void OpRTS()
		{
			uint16 PC_low = stack.Pop();
			uint16 PC_high = stack.Pop();
			regPC = ((PC_high << 8) | PC_low) + 1;
		}

		/* Special operations */
		void OpNOP()
		{
			regPC++;
		}
		
		void OpBRK() // Software interrupt
		{
			regPC += 2;
			stack.Push((regPC >> 8) & 0xFF);
			stack.Push(regPC & 0xFF);
			stack.Push(regStatus | Flag_B | Flag_Unused);
            regStatus.I = 1;
			regPC = memory.Get16At(IRQVector);
		}

		/* Flag operations */
		void SetFlagNegative(uint8& reg)
		{
            regStatus.N = reg & Bit7; // If 7th bit is 1, set negative
		}

		void SetFlagZero(uint8& reg)
        {
            regStatus.Z = reg == 0;   // If register is 0, set zero
		}

	};

} // namespace nemu
