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

		constexpr static uint8 Flag_C      = 0;
		constexpr static uint8 Flag_Z      = 1;
		constexpr static uint8 Flag_I      = 2;
		constexpr static uint8 Flag_D      = 3; // Disabled on the NES (decimal).
		constexpr static uint8 Flag_B      = 4; // Bits 4 and 5 are used to indicate whether a
		constexpr static uint8 Flag_Unused = 5; // software or hardware interrupt occured
		constexpr static uint8 Flag_V      = 6;
		constexpr static uint8 Flag_N      = 7;

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
			std::cout << "[C: " << +regStatus[Flag_C] << " | "
				      << "Z:  " << +regStatus[Flag_Z] << " | "
				      << "I:  " << +regStatus[Flag_I] << " | "
				      << "B:  " << +regStatus[Flag_B] << " | "
				      << "U:  " << +regStatus[Flag_Unused] << " | "
				      << "D:  " << +regStatus[Flag_D] << " | "
				      << "N:  " << +regStatus[Flag_N] << " | "
				      << "V:  " << +regStatus[Flag_V] << "]" << std::endl;
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
			Indirect, Relative,
			Zeropage, ZeropageX,
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
			uint16 adr = memory.Get16At(regPC + 1);
			return memory[adr];
		}
		template <>
		uint8& GetOperand<AbsoluteX>()
		{
			uint16 adr = memory.Get16At(regPC + 1) + regX;
			return memory[adr];
		}
		template <>
		uint8& GetOperand<AbsoluteY>()
		{
			uint16 adr = memory.Get16At(regPC + 1) + regY;
			return memory[adr];
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
			uint16 adr = memory.Get16At(offset);
			return memory[adr];
		}
		template <>
		uint8& GetOperand<IndirectY>() // Fetch first then add
		{
			uint8 offset = memory[regPC + 1];
			uint16 adr = memory.Get16At(offset) + regY;
			return memory[adr];
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
			std::cout << "Executing op-code: " << std::hex << "0x" << +memory[regPC] << std::endl;
			std::cout << "Offset: 0x" << regPC << std::dec << std::endl;

			switch (memory[regPC]) {
			case 0x0: { // BRK is 2 bytes even though it's implied.
				OpBRK();
				break;
			}
			case 0xA0: { // LDY Immediate
				OpLD<Immediate>(regY);
				break;
			}
			case 0xA4: { // LDY Zeropage
				OpLD<Zeropage>(regY);
				break;
			}
			case 0xB4: { // LDY Zeropage, X
				OpLD<ZeropageX>(regY);
				break;
			}
			case 0xAC: { // LDY Absolute
				OpLD<Absolute>(regY);
				break;
			}
			case 0xBC: { // LDY Absolute, X
				OpLD<AbsoluteX>(regY);
				break;
			}

			case 0xA2: { // LDX Immediate
				OpLD<Immediate>(regX);
				break;
			}
			case 0xA6: { // LDX Zeropage
				OpLD<Zeropage>(regX);
				break;
			}
			case 0xB6: { // LDX Zeropage, Y
				OpLD<ZeropageY>(regX);
				break;
			}
			case 0xAE: { // LDX Absolute
				OpLD<Absolute>(regX);
				break;
			}
			case 0xBE: { // LDX Absolute, Y
				OpLD<AbsoluteY>(regX);
				break;
			}
			case 0xEA: { // NOP
				regPC++;
				break;
			}
			case 0x18: { // CLC
				regStatus.Clear(Flag_C);
				regPC++;
				break;
			}
			case 0xD8: { // CLD
				regStatus.Clear(Flag_D);
				regPC++;
				break;
			}
			case 0x58: { // CLI
				regStatus.Clear(Flag_I);
				regPC++;
				break;
			}
			case 0xB8: { // CLV
				regStatus.Clear(Flag_V);
				regPC++;
				break;
			}
			case 0x38: { // SEC
				regStatus.Set(Flag_C);
				regPC++;
				break;
			}
			case 0xF8: { // SED
				regStatus.Set(Flag_D);
				regPC++;
				break;
			}
			case 0x78: { // SEI
				regStatus.Set(Flag_I);
				regPC++;
				break;
			}
			case 0xA9: { // LDA Immediate
				OpLD<Immediate>(regA);
				break;
			}
			case 0xA5: { // LDA Zeropage
				OpLD<Zeropage>(regA);
				break;
			}
			case 0xB5: { // LDA Zeropage, X
				OpLD<ZeropageX>(regA);
				break;
			}
			case 0xAD: { // LDA Absolute
				OpLD<Absolute>(regA);
				break;
			}
			case 0xBD: { // LDA Absolute, X
				OpLD<AbsoluteX>(regA);
				break;
			}
			case 0xB9: { // LDA Absolute, Y
				OpLD<AbsoluteY>(regA);
				break;
			}
			case 0xA1: { // LDA Indexed Indirect, X 
				OpLD<IndirectX>(regA);
				break;
			}
			case 0xB1: { // LDA Indirect Indexed, Y 
				OpLD<IndirectY>(regA);
				break;
			}
			case 0x69: { // ADC Immediate
				OpADC<Immediate>();
				break;
			}
			case 0x65: { // ADC Zeropage
				OpADC<Zeropage>();
				break;
			}
			case 0x75: { // ADC Zeropage, X
				OpADC<ZeropageX>();
				break;
			}
			case 0x6D: { // ADC Absolute
				OpADC<Absolute>();
				break;
			}
			case 0x7D: { // ADC Absolute, X
				OpADC<AbsoluteX>();
				break;
			}
			case 0x79: { // ADC Absolute, Y
				OpADC<AbsoluteY>();
				break;
			}
			case 0x61: { // ADC Indexed Indirect, X 
				OpADC<IndirectX>();
				break;
			}
			case 0x71: { // ADC Indirect Indexed, Y 
				OpADC<IndirectY>();
				break;
			}
			case 0xE9: { // SBC Immediate
				OpSBC<Immediate>();
				break;
			}
			case 0xE5: { // SBC Zeropage
				OpSBC<Zeropage>();
				break;
			}
			case 0xF5: { // SBC Zeropage, X
				OpSBC<ZeropageX>();
				break;
			}
			case 0xED: { // SBC Absolute
				OpSBC<Absolute>();
				break;
			}
			case 0xFD: { // SBC Absolute, X
				OpSBC<AbsoluteX>();
				break;
			}
			case 0xF9: { // SBC Absolute, Y
				OpSBC<AbsoluteY>();
				break;
			}
			case 0xE1: { // SBC Indexed Indirect, X 
				OpSBC<IndirectX>();
				break;
			}
			case 0xF1: { // SBC Indirect Indexed, Y 
				OpSBC<IndirectY>();
				break;
			}
			case 0x85: { // STA Zeropage
				OpST<Zeropage>(regA);
				break;
			}
			case 0x95: { // STA Zeropage, X
				OpST<ZeropageX>(regA);
				break;
			}
			case 0x8D: { // STA Absolute
				OpST<Absolute>(regA);
				break;
			}
			case 0x9D: { // STA Absolute, X
				OpST<AbsoluteX>(regA);
				break;
			}
			case 0x99: { // STA Absolute, Y
				OpST<AbsoluteY>(regA);
				break;
			}
			case 0x81: { // STA Indexed Indirect, X 
				OpST<IndirectX>(regA);
				break;
			}
			case 0x91: { // STA Indirect Indexed, Y 
				OpST<IndirectY>(regA);
				break;
			}
			case 0x86: { // STX Zeropage
				OpST<Zeropage>(regX);
				break;
			}
			case 0x96: { // STX Zeropage, Y
				OpST<ZeropageY>(regX);
				break;
			}
			case 0x8E: { // STX, Absolute
				OpST<Absolute>(regX);
				break;
			}
			case 0x84: { // STY Zeropage
				OpST<Zeropage>(regY);
				break;
			}
			case 0x94: { // STY Zeropage, X
				OpST<ZeropageX>(regY);
				break;
			}
			case 0x8C: { // STY Absolute
				OpST<Absolute>(regY);
				break;
			}
			case 0x4C: { // JMP Absolute
				regPC = memory.Get16At(regPC + 1);
				break;
			}
			case 0x6C: { // JMP Indirect (The only instruction that uses Indirect Adressing)
				uint16 offset = memory.Get16At(regPC + 1);
				regPC = memory.Get16At(offset);
				break;
			}
			case 0x20: { // JSR
				uint16 addr = memory.Get16At(regPC + 1);
				regPC += 2;
				stack.Push((regPC >> 8) & 0xFF); // Push PC_High
				stack.Push(regPC & 0xFF);        // Push PC_Low
				regPC = addr;
				break;
			}
			case 0x48: { // PHA
				stack.Push(regA);
				regPC++;
				break;
			}
			case 0x08: { // PHP
				stack.Push(regStatus | (1 << Flag_B) | (1 << Flag_Unused));
				regPC++;
				break;
			}
			case 0x68: { // PLA
				regA = stack.Pop();
				SetFlagNegative(regA);
				SetFlagZero(regA);
				regPC++;
				break;
			}
			case 0x28: { // PLP
				regStatus = stack.Pop();
				regPC++;
				break;
			}
			case 0x40: { // RTI
				regStatus = stack.Pop();
				uint16 PC_low = stack.Pop();
				uint16 PC_high = stack.Pop();
				regPC = (PC_high << 8) | PC_low;
				break;
			}
			case 0x60: { // RTS
				uint16 PC_low = stack.Pop();
				uint16 PC_high = stack.Pop();
				regPC = ((PC_high << 8) | PC_low) + 1;
				break;
			}
			case 0x10: { // BPL, Branch on result plus
				OpBRA(regStatus[Flag_N] == 0);
				break;
			}
			case 0xF0: { // BEQ
				OpBRA(regStatus[Flag_Z]);
				break;
			}
			case 0x90: { // BCC
				OpBRA(regStatus[Flag_C] == 0);
				break;
			}
			case 0xB0: { // BCS
				OpBRA(regStatus[Flag_C]);
				break;
			}
			case 0x30: { // BMI
				OpBRA(regStatus[Flag_N]);
				break;
			}
			case 0xD0: { // BNE
				OpBRA(regStatus[Flag_Z] == 0);
				break;
			}
			case 0x50: { // BVC
				OpBRA(regStatus[Flag_V] == 0);
				break;
			}
			case 0x70: { // BVS
				OpBRA(regStatus[Flag_V]);
				break;
			}
			case 0x24: { // BIT Zeropage
				OpBIT<Zeropage>();
				break;
			}
			case 0x2C: { // BIT Absolute
				OpBIT<Absolute>();
				break;
			}
			case 0xCA: { // DEX, Decrement X by 1
				regX--;
				SetFlagNegative(regX);
				SetFlagZero(regX);
				regPC++;
				break;
			}
			case 0x88: { // DEY, Decrement Y by 1
				regY--;
				regStatus.Set(Flag_Z, regY == 0);
				regStatus.Set(Flag_N, regY & Bit7);
				regPC++;
				break;
			}
			case 0xC6: { // DEC Zeropage
				OpDEC<Zeropage>();
				break;
			}
			case 0xD6: { // DEC Zeropage, X
				OpDEC<ZeropageX>();
				break;
			}
			case 0xCE: { // DEC Absolute
				OpDEC<Absolute>();
				break;
			}
			case 0xDE: { // DEC Absolute, X
				OpDEC<AbsoluteX>();
				break;
			}
			case 0x8A: { // TXA (Transfer X -> A)
				regA = regX;
				SetFlagNegative(regA);
				SetFlagZero(regA);
				regPC++;
				break;
			}
			case 0xAA: { // TAX (Transfer A -> X)
				regX = regA;
				SetFlagNegative(regX);
				SetFlagZero(regX);
				regPC++;
				break;
			}
			case 0xA8: { // TAY (Transfer A -> Y)
				regY = regA;
				SetFlagNegative(regY);
				SetFlagZero(regY);
				regPC++;
				break;
			}
			case 0xBA: { // TSX (Transfer SP -> X)
				regX = stack.GetSP();
				SetFlagNegative(regX);
				SetFlagZero(regX);
				regPC++;
				break;
			}
			case 0x9A: { // TXS (Transfer X -> SP)
				stack.SetSP(regX);
				regPC++;
				break;
			}
			case 0x98: { // TYA (Transfer Y -> A)
				regA = regY;
				SetFlagNegative(regA);
				SetFlagZero(regA);
				regPC++;
				break;
			}
			case 0x0A: { // ASL Accumulator (shift left)
				OpASL<Implied>(regA);
				break;
			}
			case 0x06: { // ASL Zeropage
				uint8& oper = GetOperand<Zeropage>();
				OpASL<Zeropage>(oper);
				break;
			}
			case 0x16: { // ASL Zeropage, X
				uint8& oper = GetOperand<ZeropageX>();
				OpASL<ZeropageX>(oper);
				break;
			}
			case 0x0E: { // ASL Absolute
				uint8& oper = GetOperand<Absolute>();
				OpASL<Absolute>(oper);
				break;
			}
			case 0x1E: { // ASL Absolute, X
				uint8& oper = GetOperand<AbsoluteX>();
				OpASL<AbsoluteX>(oper);
				break;
			}
			case 0x4A: { // LSR Accumulator (shift right)
				OpLSR<Implied>(regA);
				break;
			}
			case 0x46: { // LSR Zeropage
				uint8& oper = GetOperand<Zeropage>();
				OpLSR<Zeropage>(oper);
				break;
			}
			case 0x56: { // LSR Zeropage, X
				uint8& oper = GetOperand<ZeropageX>();
				OpLSR<ZeropageX>(oper);
				break;
			}
			case 0x4E: { // LSR Absolute
				uint8& oper = GetOperand<Absolute>();
				OpLSR<Absolute>(oper);
				break;
			}
			case 0x5E: { // LSR Absolute, X
				uint8& oper = GetOperand<AbsoluteX>();
				OpLSR<AbsoluteX>(oper);
				break;
			}
			case 0x2A: { // ROL Accumulator (rotate left)
				OpROL<Implied>(regA);
				break;
			}
			case 0x26: { // ROL Zeropage
				uint8& oper = GetOperand<Zeropage>();
				OpROL<Zeropage>(oper);
				break;
			}
			case 0x36: { // ROL Zeropage, X
				uint8& oper = GetOperand<ZeropageX>();
				OpROL<ZeropageX>(oper);
				break;
			}
			case 0x2E: { // ROL Absolute
				uint8& oper = GetOperand<Absolute>();
				OpROL<Absolute>(oper);
				break;
			}
			case 0x3E: { // ROL Absolute, X
				uint8& oper = GetOperand<AbsoluteX>();
				OpROL<AbsoluteX>(oper);
				break;
			}
			case 0x6A: { // ROR Accumulator (rotate right)
				OpROR<Implied>(regA);
				break;
			}
			case 0x66: { // ROR Zeropage
				uint8& oper = GetOperand<Zeropage>();
				OpROR<Zeropage>(oper);
				break;
			}
			case 0x76: { // ROR Zeropage, X
				uint8& oper = GetOperand<ZeropageX>();
				OpROR<ZeropageX>(oper);
				break;
			}
			case 0x6E: { // ROR Absolute
				uint8& oper = GetOperand<Absolute>();
				OpROR<Absolute>(oper);
				break;
			}
			case 0x7E: { // ROR Absolute, X
				uint8& oper = GetOperand<AbsoluteX>();
				OpROR<AbsoluteX>(oper);
				break;
			}
			case 0x29: { // AND Immediate
				OpAND<Immediate>();
				break;
			}
			case 0x25: { // AND Zeropage
				OpAND<Zeropage>();
				break;
			}
			case 0x35: { // AND Zeropage, X
				OpAND<ZeropageX>();
				break;
			}
			case 0x2D: { // AND Absolute
				OpAND<Absolute>();
				break;
			}
			case 0x3D: { // AND Absolute, X
				OpAND<AbsoluteX>();
				break;
			}
			case 0x39: { // AND Absolute, Y
				OpAND<AbsoluteY>();
				break;
			}
			case 0x21: { // AND Indexed Indirect, X 
				OpAND<IndirectX>();
				break;
			}
			case 0x31: { // AND Indirect Indexed, Y 
				OpAND<IndirectY>();
				break;
			}
			case 0xC9: { // CMP Immediate
				OpCMP<Immediate>(regA);
				break;
			}
			case 0xC5: { // CMP Zeropage
				OpCMP<Zeropage>(regA);
				break;
			}
			case 0xD5: { // CMP Zeropage, X
				OpCMP<ZeropageX>(regA);
				break;
			}
			case 0xCD: { // CMP Absolute
				OpCMP<Absolute>(regA);
				break;
			}
			case 0xDD: { // CMP Absolute, X
				OpCMP<AbsoluteX>(regA);
				break;
			}
			case 0xD9: { // CMP Absolute, Y
				OpCMP<AbsoluteY>(regA);
				break;
			}
			case 0xC1: { // CMP Indexed Indirect, X 
				OpCMP<IndirectX>(regA);
				break;
			}
			case 0xD1: { // CMP Indirect Indexed, Y 
				OpCMP<IndirectY>(regA);
				break;
			}
			case 0xE0: { // CPX Immediate
				OpCMP<Immediate>(regX);
				break;
			}
			case 0xE4: { // CPX Zeropage
				OpCMP<Zeropage>(regX);
				break;
			}
			case 0xEC: { // CPX Absolute
				OpCMP<Absolute>(regX);
				break;
			}
			case 0xC0: { // CPY Immediate
				OpCMP<Immediate>(regY);
				break;
			}
			case 0xC4: { // CPY Zeropage
				OpCMP<Zeropage>(regY);
				break;
			}
			case 0xCC: { // CPY Absolute
				OpCMP<Absolute>(regY);
				break;
			}
			case 0x09: { // ORA Immediate
				OpORA<Immediate>();
				break;
			}
			case 0x05: { // ORA Zeropage
				OpORA<Zeropage>();
				break;
			}
			case 0x15: { // ORA Zeropage, X
				OpORA<ZeropageX>();
				break;
			}
			case 0x0D: { // ORA Absolute
				OpORA<Absolute>();
				break;
			}
			case 0x1D: { // ORA Absolute, X
				OpORA<AbsoluteX>();
				break;
			}
			case 0x19: { // ORA Absolute, Y
				OpORA<AbsoluteY>();
				break;
			}
			case 0x01: { // ORA Indexed Indirect, X 
				OpORA<IndirectX>();
				break;
			}
			case 0x11: { // ORA Indirect Indexed, Y 
				OpORA<IndirectY>();
				break;
			}
			case 0x49: { // EOR Immediate
				OpEOR<Immediate>();
				break;
			}
			case 0x45: { // EOR Zeropage
				OpEOR<Zeropage>();
				break;
			}
			case 0x55: { // EOR Zeropage, X
				OpEOR<ZeropageX>();
				break;
			}
			case 0x4D: { // EOR Absolute
				OpEOR<Absolute>();
				break;
			}
			case 0x5D: { // EOR Absolute, X
				OpEOR<AbsoluteX>();
				break;
			}
			case 0x59: { // EOR Absolute, Y
				OpEOR<AbsoluteY>();
				break;
			}
			case 0x41: { // EOR Indexed Indirect, X 
				OpEOR<IndirectX>();
				break;
			}
			case 0x51: { // EOR Indirect Indexed, Y 
				OpEOR<IndirectY>();
				break;
			}
			case 0xE6: { // INC Zeropage
				OpINC<Zeropage>();
				break;
			}
			case 0xF6: { // INC Zeropage, X
				OpINC<ZeropageX>();
				break;
			}
			case 0xEE: { // INC Absolute
				OpINC<Absolute>();
				break;
			}
			case 0xFE: { // INC Absolute, X
				OpINC<AbsoluteX>();
				break;
			}
			case 0xE8: { // INX
				regX++;
				SetFlagNegative(regX);
				SetFlagZero(regX);
				regPC++;
				break;
			}
			case 0xC8: { // INY
				regY++;
				SetFlagNegative(regY);
				SetFlagZero(regY);
				regPC++;
				break;
			}

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
		void OpROL(uint8& reg)
		{
			uint16 temp = reg;                                       // Holds carry in bit 8
			temp <<= 1;                                              // Shifts left one bit
			temp = regStatus[Flag_C] ? temp | Bit0 : temp & ~Bit0;   // Changes bit 0 to whatever carry is
			regStatus.Set(Flag_C, temp & Bit8);                      // Sets carry flag to whatever bit 8 is
			reg = temp & 0xFF;
			SetFlagNegative(reg);
			SetFlagZero(reg);
			regPC += InstructionSize<Mode>();
		}
		template <Addressmode Mode>
		void OpROR(uint8& reg)
		{
			uint16 temp = reg;
			temp = regStatus[Flag_C] ? temp | Bit8 : temp & ~Bit8;
			regStatus.Set(Flag_C, temp & Bit0);
			temp >>= 1;
			reg = temp & 0xFF;
			SetFlagNegative(reg);
			SetFlagZero(reg);
			regPC += InstructionSize<Mode>();
		}
		template <Addressmode Mode>
		void OpASL(uint8& reg)
		{
			regStatus.Set(Flag_C, reg & Bit7);
			reg <<= 1;
			SetFlagNegative(reg);
			SetFlagZero(reg);
			regPC += InstructionSize<Mode>();
		}
		template <Addressmode Mode>
		void OpLSR(uint8& reg)
		{
			regStatus.Set(Flag_C, reg & Bit0);
			reg >>= 1;
			regStatus.Set(Flag_Z, reg == 0);
			regStatus.Clear(Flag_N);
			regPC += InstructionSize<Mode>();
		}
		template <Addressmode Mode>
		void OpDEC()
		{
			uint8& oper = GetOperand<Mode>();
			oper--;
			regStatus.Set(Flag_Z, oper == 0);
			regStatus.Set(Flag_N, oper & Bit7);
			regPC += InstructionSize<Mode>();
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
		void OpBRA(bool condition) // Branch operations
		{
			int8 oper = GetOperand<Immediate>();
			regPC += condition ? oper + InstructionSize<Immediate>() : InstructionSize<Immediate>();
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
		void OpINC() // Increase operations
		{
			uint8& oper = GetOperand<Mode>();
			oper++;
			SetFlagNegative(oper);
			SetFlagZero(oper);
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
		void OpBRK()
		{
			regPC += 2;
			stack.Push((regPC >> 8) & 0xFF);
			stack.Push(regPC & 0xFF);
			stack.Push(regStatus | (1 << Flag_B) | (1 << Flag_Unused));
			regStatus.Set(Flag_I);
			regPC = memory.Get16At(IRQVector);
		}

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