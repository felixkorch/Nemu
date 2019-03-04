#pragma once
#include "Nemu/Stack.h"
#include "Nemu/NESMemory.h"
#include "Nemu/Mapper/NROM256Mapper.h"
#include "Nemu/Bitset.h"
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
		typedef unsigned int uint;
		typedef std::int32_t int32;
		typedef std::uint32_t uint32;
		typedef std::uint16_t uint16;
		typedef std::uint8_t uint8;
		typedef std::int8_t int8;

		uint8 regX;
		uint8 regY;
		uint8 regA;

		uint16 regPC;
		Bitset<8> regStatus;

		Memory& memory;
		Stack<typename Memory::iterator> stack;

		constexpr static unsigned int Flag_C      = 0;
		constexpr static unsigned int Flag_Z      = 1;
		constexpr static unsigned int Flag_I      = 2;
		constexpr static unsigned int Flag_D      = 3; // Disabled on the NES (decimal).
		constexpr static unsigned int Flag_B      = 4; // Bits 4 and 5 are used to indicate whether a
		constexpr static unsigned int Flag_Unused = 5; // software or hardware interrupt occured
		constexpr static unsigned int Flag_V      = 6;
		constexpr static unsigned int Flag_N      = 7;

		constexpr static uint8  Bit0 = (1 << 0);
		constexpr static uint8  Bit1 = (1 << 1);
		constexpr static uint8  Bit2 = (1 << 2);
		constexpr static uint8  Bit3 = (1 << 3);
		constexpr static uint8  Bit4 = (1 << 4);
		constexpr static uint8  Bit5 = (1 << 5);
		constexpr static uint8  Bit6 = (1 << 6);
		constexpr static uint8  Bit7 = (1 << 7);
		constexpr static uint16 Bit8 = (1 << 8);

		constexpr static uint16 ResetVector = 0xFFFC;
		constexpr static uint16 NMIVector   = 0xFFFA;
		constexpr static uint16 IRQVector   = 0xFFFE;

	public:
		CPU(Memory& mem)
			: regX(0),
			  regY(0),
			  regA(0),
			  regPC(0),
			  regStatus{},
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
			stack.Push(regStatus & ~(1 << Flag_B) | (1 << Flag_Unused));
			regStatus.Set(Flag_I);
			regPC = memory.Get16At(NMIVector);
		}

		void InvokeIRQ()
		{
			if (!regStatus[Flag_I]) {
				stack.Push((regPC >> 8) & 0xFF);
				stack.Push(regPC & 0xFF);
				stack.Push(regStatus & ~(1 << Flag_B) | (1 << Flag_Unused));
				regStatus.Set(Flag_I);
				regPC = memory.Get16At(IRQVector);
			}
		}

		void Execute()
		{
			Decode();
		}

		void PrintFlags()
		{
			std::cout << "[C: " << +regStatus[Flag_C]      << " | "
				      << "Z:  " << +regStatus[Flag_Z]      << " | "
				      << "I:  " << +regStatus[Flag_I]      << " | "
				      << "B:  " << +regStatus[Flag_B]      << " | "
				      << "U:  " << +regStatus[Flag_Unused] << " | "
				      << "D:  " << +regStatus[Flag_D]      << " | "
				      << "N:  " << +regStatus[Flag_N]      << " | "
				      << "V:  " << +regStatus[Flag_V]      << "]  " << std::endl;
		}

		void PrintRegisters()
		{
			std::cout << "A: " << +regA << std::endl;
			std::cout << "X: " << +regX << std::endl;
			std::cout << "Y: " << +regY << std::endl;
		}

		/// -------------------------------------------PRIVATE FUNCTIONS------------------------------------------------------- ///

	private:

		enum Addressmode {
			Immediate, Absolute,
			AbsoluteX, AbsoluteY,
			Indirect,  Relative,
			Zeropage,  ZeropageX,
			ZeropageY, IndirectX,
			IndirectY, Implied
		};

		template <Addressmode Mode>
		uint8& GetOperand();

		template <Addressmode Mode>
		constexpr unsigned int InstructionSize();
		template<> constexpr unsigned int InstructionSize <Immediate>() { return 2; }
		template<> constexpr unsigned int InstructionSize <Absolute>()  { return 3; }
		template<> constexpr unsigned int InstructionSize <AbsoluteX>() { return 3; }
		template<> constexpr unsigned int InstructionSize <AbsoluteY>() { return 3; }
		template<> constexpr unsigned int InstructionSize <Indirect>()  { return 3; }
		template<> constexpr unsigned int InstructionSize <Relative>()  { return 2; }
		template<> constexpr unsigned int InstructionSize <Zeropage>()  { return 2; }
		template<> constexpr unsigned int InstructionSize <ZeropageX>() { return 2; }
		template<> constexpr unsigned int InstructionSize <ZeropageY>() { return 2; }
		template<> constexpr unsigned int InstructionSize <IndirectX>() { return 2; }
		template<> constexpr unsigned int InstructionSize <IndirectY>() { return 2; }
		template<> constexpr unsigned int InstructionSize <Implied>()   { return 1; }

		template <>
		uint8& GetOperand<Immediate>()
		{
			return memory[regPC + 1];
		}
		template <>
		uint8& GetOperand<Absolute>()
		{
			uint16 addr = memory.Get16At(regPC + 1);
			return memory[addr];
		}
		template <>
		uint8& GetOperand<AbsoluteX>()
		{
			uint16 addr = memory.Get16At(regPC + 1) + regX;
			return memory[addr];
		}
		template <>
		uint8& GetOperand<AbsoluteY>()
		{
			uint16 addr = memory.Get16At(regPC + 1) + regY;
			return memory[addr];
		}
		template <>
		uint8& GetOperand<Relative>()
		{
			return memory[regPC + 1];
		}
		template <>
		uint8& GetOperand<IndirectX>() // Add first then fetch
		{
			uint8 offset = memory[regPC + 1] + regX;
			uint16 addr = memory.Get16At(offset);
			return memory[addr];
		}
		template <>
		uint8& GetOperand<IndirectY>() // Fetch first then add
		{
			uint8 offset = memory[regPC + 1];
			uint16 addr = memory.Get16At(offset) + regY;
			return memory[addr];
		}
		template <>
		uint8& GetOperand<Zeropage>()
		{
			uint8 offset = memory[regPC + 1];
			return memory[offset];
		}
		template <>
		uint8& GetOperand<ZeropageX>()
		{
			uint8 offset = memory[regPC + 1] + regX;
			return memory[offset];
		}
		template <>
		uint8& GetOperand<ZeropageY>()
		{
			uint8 offset = memory[regPC + 1] + regY;
			return memory[offset];
		}

		void Decode() // Fetches & decodes an instruction
		{
			std::cout << "0x" << std::hex << regPC << std::endl;
			switch (memory[regPC]) {
			case 0x00: OpBRK();                       break;
			case 0xA0: OpLD<Immediate>(regY);         break;
			case 0xA4: OpLD<Zeropage>(regY);          break;
			case 0xB4: OpLD<ZeropageX>(regY);         break;
			case 0xAC: OpLD<Absolute>(regY);          break;
			case 0xBC: OpLD<AbsoluteX>(regY);         break;
			case 0xA2: OpLD<Immediate>(regX);         break;
			case 0xA6: OpLD<Zeropage>(regX);          break;
			case 0xB6: OpLD<ZeropageY>(regX);         break;
			case 0xAE: OpLD<Absolute>(regX);          break;
			case 0xBE: OpLD<AbsoluteY>(regX);         break;
			case 0xEA: OpNOP();                       break;
			case 0x18: OpCLR(Flag_C);                 break;
			case 0xD8: OpCLR(Flag_D);                 break;
			case 0x58: OpCLR(Flag_I);                 break;
			case 0xB8: OpCLR(Flag_V);                 break;
			case 0x38: OpSET(Flag_C);                 break;
			case 0xF8: OpSET(Flag_D);                 break;
			case 0x78: OpSET(Flag_I);                 break;
			case 0xA9: OpLD<Immediate>(regA);         break;
			case 0xA5: OpLD<Zeropage>(regA);          break;
			case 0xB5: OpLD<ZeropageX>(regA);         break;
			case 0xAD: OpLD<Absolute>(regA);          break;
			case 0xBD: OpLD<AbsoluteX>(regA);         break;
			case 0xB9: OpLD<AbsoluteY>(regA);         break;
			case 0xA1: OpLD<IndirectX>(regA);         break;
			case 0xB1: OpLD<IndirectY>(regA);         break;
			case 0x69: OpADC<Immediate>();            break;
			case 0x65: OpADC<Zeropage>();             break;
			case 0x75: OpADC<ZeropageX>();            break;
			case 0x6D: OpADC<Absolute>();             break;
			case 0x7D: OpADC<AbsoluteX>();            break;
			case 0x79: OpADC<AbsoluteY>();            break;
			case 0x61: OpADC<IndirectX>();            break;
			case 0x71: OpADC<IndirectY>();            break;
			case 0xE9: OpSBC<Immediate>();            break;
			case 0xE5: OpSBC<Zeropage>();             break;
			case 0xF5: OpSBC<ZeropageX>();            break;
			case 0xED: OpSBC<Absolute>();             break;
			case 0xFD: OpSBC<AbsoluteX>();            break;
			case 0xF9: OpSBC<AbsoluteY>();            break;
			case 0xE1: OpSBC<IndirectX>();            break;
			case 0xF1: OpSBC<IndirectY>();            break;
			case 0x85: OpST<Zeropage>(regA);          break;
			case 0x95: OpST<ZeropageX>(regA);         break;
			case 0x8D: OpST<Absolute>(regA);          break;
			case 0x9D: OpST<AbsoluteX>(regA);         break;
			case 0x99: OpST<AbsoluteY>(regA);         break;
			case 0x81: OpST<IndirectX>(regA);         break;
			case 0x91: OpST<IndirectY>(regA);         break;
			case 0x86: OpST<Zeropage>(regX);          break;
			case 0x96: OpST<ZeropageY>(regX);         break;
			case 0x8E: OpST<Absolute>(regX);          break;
			case 0x84: OpST<Zeropage>(regY);          break;
			case 0x94: OpST<ZeropageX>(regY);         break;
			case 0x8C: OpST<Absolute>(regY);          break;
			case 0x4C: OpJMP<Absolute>();             break;
			case 0x6C: OpJMP<Indirect>();             break;
			case 0x20: OpJSR();                       break;
			case 0x48: OpPHA();                       break;
			case 0x08: OpPHP();                       break;
			case 0x68: OpPLA();                       break;
			case 0x28: OpPLP();                       break;
			case 0x40: OpRTI();                       break;
			case 0x60: OpRTS();                       break;
			case 0x10: OpBRA(!regStatus[Flag_N]);     break;
			case 0xF0: OpBRA(regStatus[Flag_Z]);      break;
			case 0x90: OpBRA(!regStatus[Flag_C]);     break;
			case 0xB0: OpBRA(regStatus[Flag_C]);      break;
			case 0x30: OpBRA(regStatus[Flag_N]);      break;
			case 0xD0: OpBRA(!regStatus[Flag_Z]);     break;
			case 0x50: OpBRA(!regStatus[Flag_V]);     break;
			case 0x70: OpBRA(regStatus[Flag_V]);      break;
			case 0x24: OpBIT<Zeropage>();             break;
			case 0x2C: OpBIT<Absolute>();             break;
			case 0x88: OpDEY();                       break;
			case 0xCA: OpDEX();                       break;
			case 0xC6: OpDEC<Zeropage>();             break;
			case 0xD6: OpDEC<ZeropageX>();            break;
			case 0xCE: OpDEC<Absolute>();             break;
			case 0xDE: OpDEC<AbsoluteX>();            break;
			case 0x8A: OpTXA();                       break;
			case 0xAA: OpTAX();                       break;
			case 0xA8: OpTAY();                       break;
			case 0xBA: OpTSX();                       break;
			case 0x9A: OpTXS();                       break;
			case 0x98: OpTYA();                       break;
			case 0x0A: OpASL<Implied>();              break;
			case 0x06: OpASL<Zeropage>();             break;
			case 0x16: OpASL<ZeropageX>();            break;
			case 0x0E: OpASL<Absolute>();             break;
			case 0x1E: OpASL<AbsoluteX>();            break;
			case 0x4A: OpLSR<Implied>();              break;
			case 0x46: OpLSR<Zeropage>();             break;
			case 0x56: OpLSR<ZeropageX>();            break;
			case 0x4E: OpLSR<Absolute>();             break;
			case 0x5E: OpLSR<AbsoluteX>();            break;
			case 0x2A: OpROL<Implied>();              break;
			case 0x26: OpROL<Zeropage>();             break;
			case 0x36: OpROL<ZeropageX>();            break;
			case 0x2E: OpROL<Absolute>();             break;
			case 0x3E: OpROL<AbsoluteX>();            break;
			case 0x6A: OpROR<Implied>();              break;
			case 0x66: OpROR<Zeropage>();             break;
			case 0x76: OpROR<ZeropageX>();            break;
			case 0x6E: OpROR<Absolute>();             break;
			case 0x7E: OpROR<AbsoluteX>();            break;
			case 0x29: OpAND<Immediate>();            break;
			case 0x25: OpAND<Zeropage>();             break;
			case 0x35: OpAND<ZeropageX>();            break;
			case 0x2D: OpAND<Absolute>();             break;
			case 0x3D: OpAND<AbsoluteX>();            break;
			case 0x39: OpAND<AbsoluteY>();            break;
			case 0x21: OpAND<IndirectX>();            break;
			case 0x31: OpAND<IndirectY>();            break;
			case 0xC9: OpCMP<Immediate>(regA);        break;
			case 0xC5: OpCMP<Zeropage>(regA);         break;
			case 0xD5: OpCMP<ZeropageX>(regA);        break;
			case 0xCD: OpCMP<Absolute>(regA);         break;
			case 0xDD: OpCMP<AbsoluteX>(regA);        break;
			case 0xD9: OpCMP<AbsoluteY>(regA);        break;
			case 0xC1: OpCMP<IndirectX>(regA);        break;
			case 0xD1: OpCMP<IndirectY>(regA);        break;
			case 0xE0: OpCMP<Immediate>(regX);        break;
			case 0xE4: OpCMP<Zeropage>(regX);         break;
			case 0xEC: OpCMP<Absolute>(regX);         break;
			case 0xC0: OpCMP<Immediate>(regY);        break;
			case 0xC4: OpCMP<Zeropage>(regY);         break;
			case 0xCC: OpCMP<Absolute>(regY);         break;
			case 0x09: OpORA<Immediate>();            break;
			case 0x05: OpORA<Zeropage>();             break;
			case 0x15: OpORA<ZeropageX>();            break;
			case 0x0D: OpORA<Absolute>();             break;
			case 0x1D: OpORA<AbsoluteX>();            break;
			case 0x19: OpORA<AbsoluteY>();            break;
			case 0x01: OpORA<IndirectX>();            break;
			case 0x11: OpORA<IndirectY>();            break;
			case 0x49: OpEOR<Immediate>();            break;
			case 0x45: OpEOR<Zeropage>();             break;
			case 0x55: OpEOR<ZeropageX>();            break;
			case 0x4D: OpEOR<Absolute>();             break;
			case 0x5D: OpEOR<AbsoluteX>();            break;
			case 0x59: OpEOR<AbsoluteY>();            break;
			case 0x41: OpEOR<IndirectX>();            break;
			case 0x51: OpEOR<IndirectY>();            break;
			case 0xE6: OpINC<Zeropage>();             break;
			case 0xF6: OpINC<ZeropageX>();            break;
			case 0xEE: OpINC<Absolute>();             break;
			case 0xFE: OpINC<AbsoluteX>();            break;
			case 0xE8: OpINX();                       break;
			case 0xC8: OpINY();                       break;
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

		template <Addressmode Mode>
		void OpADC()
		{
			uint8& oper = GetOperand<Mode>();
			uint result = regA + oper + regStatus[Flag_C];
			bool overflow = !((regA ^ oper) & Bit7) && ((regA ^ result) & Bit7);
			regStatus.Set(Flag_Z, (result & 0xFF) == 0);
			regStatus.Set(Flag_C, result > 0xFF);	           
			regStatus.Set(Flag_V, overflow);
			regStatus.Set(Flag_N, result & Bit7);
			regA = result & 0xFF;
			regPC += InstructionSize<Mode>();
		}

		template <Addressmode Mode>
		void OpSBC()
		{
			uint8& oper = GetOperand<Mode>();
			uint result = regA - oper - !regStatus[Flag_C];
			bool overflow = ((regA ^ result) & Bit7) && ((regA ^ oper) & Bit7);
			regStatus.Set(Flag_Z, (result & 0xFF) == 0);
			regStatus.Set(Flag_C, result < Bit8);
			regStatus.Set(Flag_V, overflow);
			regStatus.Set(Flag_N, result & Bit7);
			regA = result & 0xFF;
			regPC += InstructionSize<Mode>();
		}
		template <Addressmode Mode>
		void OpCMP(uint8& reg)
		{
			uint8& oper = GetOperand<Mode>();
			uint result = reg - oper;
			regStatus.Set(Flag_Z, reg == oper);
			regStatus.Set(Flag_N, result & Bit7);
			regStatus.Set(Flag_C, result < Bit8);
			regPC += InstructionSize<Mode>();
		}
		template <Addressmode Mode>
		void OpAND()
		{
			uint8& oper = GetOperand<Mode>();
			regA &= oper;
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC += InstructionSize<Mode>();
		}
		template <Addressmode Mode>
		void OpEOR()
		{
			uint8& oper = GetOperand<Mode>();
			regA ^= oper;
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC += InstructionSize<Mode>();
		}
		template <Addressmode Mode>
		void OpROL()
		{
			uint8& oper = GetOperand<Mode>();
			uint16 temp = oper;                                      // Holds carry in bit 8
			temp <<= 1;                                              // Shifts left one bit
			temp = regStatus[Flag_C] ? temp | Bit0 : temp & ~Bit0;   // Changes bit 0 to whatever carry is
			regStatus.Set(Flag_C, temp & Bit8);                      // Sets carry flag to whatever bit 8 is
			oper = temp & 0xFF;
			SetFlagNegative(oper);
			SetFlagZero(oper);
			regPC += InstructionSize<Mode>();
		}
		template <>
		void OpROL<Implied>() // Special case for Accumulator ROL
		{
			uint16 temp = regA;
			temp <<= 1;
			temp = regStatus[Flag_C] ? temp | Bit0 : temp & ~Bit0;
			regStatus.Set(Flag_C, temp & Bit8);
			regA = temp & 0xFF;
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC += InstructionSize<Implied>();
		}
		template <Addressmode Mode>
		void OpROR()
		{
			uint8& oper = GetOperand<Mode>();
			uint16 temp = oper;
			temp = regStatus[Flag_C] ? temp | Bit8 : temp & ~Bit8;
			regStatus.Set(Flag_C, temp & Bit0);
			temp >>= 1;
			oper = temp & 0xFF;
			SetFlagNegative(oper);
			SetFlagZero(oper);
			regPC += InstructionSize<Mode>();
		}
		template <>
		void OpROR<Implied>() // Special case for Accumulator ROR
		{
			uint16 temp = regA;
			temp = regStatus[Flag_C] ? temp | Bit8 : temp & ~Bit8;
			regStatus.Set(Flag_C, temp & Bit0);
			temp >>= 1;
			regA = temp & 0xFF;
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC += InstructionSize<Implied>();
		}
		template <Addressmode Mode>
		void OpASL()
		{
			uint8& oper = GetOperand<Mode>();
			regStatus.Set(Flag_C, oper & Bit7);
			oper <<= 1;
			SetFlagNegative(oper);
			SetFlagZero(oper);
			regPC += InstructionSize<Mode>();
		}
		template <>
		void OpASL<Implied>() // Special case for Accumulator ASL
		{
			regStatus.Set(Flag_C, regA & Bit7);
			regA <<= 1;
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC += InstructionSize<Implied>();
		}
		template <Addressmode Mode>
		void OpLSR()
		{
			uint8& oper = GetOperand<Mode>();
			regStatus.Set(Flag_C, oper & Bit0);
			oper >>= 1;
			regStatus.Set(Flag_Z, oper == 0);
			regStatus.Clear(Flag_N);
			regPC += InstructionSize<Mode>();
		}
		template <>
		void OpLSR<Implied>() // Special case for Accumulator LSR
		{
			regStatus.Set(Flag_C, regA & Bit0);
			regA >>= 1;
			regStatus.Set(Flag_Z, regA == 0);
			regStatus.Clear(Flag_N);
			regPC += InstructionSize<Implied>();
		}

		/* Decrease Operations */
		template <Addressmode Mode>
		void OpDEC()
		{
			uint8& oper = GetOperand<Mode>();
			oper--;
			regStatus.Set(Flag_Z, oper == 0);
			regStatus.Set(Flag_N, oper & Bit7);
			regPC += InstructionSize<Mode>();
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
			regStatus.Set(Flag_Z, regY == 0);
			regStatus.Set(Flag_N, regY & Bit7);
			regPC++;
		}

		/* Increase Operations */
		template <Addressmode Mode>
		void OpINC() // Increase operations
		{
			uint8& oper = GetOperand<Mode>();
			oper++;
			SetFlagNegative(oper);
			SetFlagZero(oper);
			regPC += InstructionSize<Mode>();
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

		template <Addressmode Mode>
		void OpLD(uint8& reg) // Load operations
		{
			uint8& oper = GetOperand<Mode>();
			reg = oper;
			SetFlagNegative(reg);
			SetFlagZero(reg);
			regPC += InstructionSize<Mode>();
		}
		template <Addressmode Mode>
		void OpST(uint8& reg) // Store operations
		{
			uint8& oper = GetOperand<Mode>();
			oper = reg;
			regPC += InstructionSize<Mode>();
		}
		template <Addressmode Mode>
		void OpORA()
		{
			uint8& oper = GetOperand<Mode>();
			regA |= oper;
			SetFlagNegative(regA);
			SetFlagZero(regA);
			regPC += InstructionSize<Mode>();
		}
		template <Addressmode Mode>
		void OpBIT()
		{
			uint8& oper = GetOperand<Mode>();
			regStatus.Set(Flag_N, oper & Bit7);
			regStatus.Set(Flag_V, oper & Bit6);
			regStatus.Set(Flag_Z, (oper & regA) == 0);
			regPC += InstructionSize<Mode>();
		}
		void OpCLR(unsigned int flag)
		{
			regStatus.Clear(flag);
			regPC++;
		}
		void OpSET(unsigned int flag)
		{
			regStatus.Set(flag);
			regPC++;
		}

		/* Jump operations */
		template <Addressmode Mode> void OpJMP();
		template <>
		void OpJMP<Absolute>()
		{
			regPC = memory.Get16At(regPC + 1);
		}
		template <>
		void OpJMP<Indirect>()
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
		void OpBRA(bool condition) // Branch operations
		{
			int8 oper = GetOperand<Immediate>();
			regPC += condition ? oper + InstructionSize<Immediate>() : InstructionSize<Immediate>();
		}

		/* Stack operations */
		void OpPHA()
		{
			stack.Push(regA);
			regPC++;
		}

		void OpPHP()
		{
			stack.Push(regStatus | (1 << Flag_B) | (1 << Flag_Unused));
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
			stack.Push(regStatus | (1 << Flag_B) | (1 << Flag_Unused));
			regStatus.Set(Flag_I);
			regPC = memory.Get16At(IRQVector);
		}

		/* Flag operations */
		void SetFlagNegative(uint8& reg)
		{
			regStatus.Set(Flag_N, reg & Bit7);   // If 7th bit is 1, set negative
		}
		void SetFlagZero(uint8& reg)
		{
			regStatus.Set(Flag_Z, reg == 0);  	 // If register is 0, set zero
		}

	};

} // namespace nemu