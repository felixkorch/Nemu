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
		bool running;

		// Status bits
		constexpr static uint8 Flag_C         = 0;
		constexpr static uint8 Flag_Z         = 1;
		constexpr static uint8 Flag_I         = 2;
		constexpr static uint8 Flag_D         = 3; // Disabled on the NES (decimal).
		constexpr static uint8 Flag_B         = 4; // Bits 4 and 5 are used to indicate whether a -
		constexpr static uint8 Flag_Unused    = 5; // software or hardware interrupt occured
		constexpr static uint8 Flag_V         = 6;
		constexpr static uint8 Flag_N         = 7;

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
			  memory(mem),
			  running(true)
		{
			regPC = memory.Get16At(ResetVector);  // Load PC with the reset vector.
		}

		/// -------------------------------------------PUBLIC FUNCTIONS------------------------------------------------------- ///

		void ForceStop()
		{
			running = false;
		}

		void InvokeNMI()
		{
			stack.Push((regPC >> 8) & 0xFF); // Push PC_High
			stack.Push(regPC & 0xFF);        // Push PC_Low
			stack.Push(regStatus);
			regStatus.Set(Flag_I);
			regPC = memory.Get16At(NMIVector);
		}

		void InvokeIRQ()
		{
			stack.Push((regPC >> 8) & 0xFF); // Push PC_High
			stack.Push(regPC & 0xFF);        // Push PC_Low
			stack.Push(regStatus);
			regStatus.Set(Flag_I);
			regPC = memory.Get16At(IRQVector);
		}

		Memory& GetMemory()
		{
			return memory;
		}

		void Execute(int n)
		{
			for (int i = 0; i < n; i++) {
				if (!Decode()) {
					std::cout << "Error: Illegal op-code" << std::endl;
					return;
				}
			}
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
				      << "O:  " << +regStatus[Flag_V] << "]" << std::endl;
		}

		void PrintRegisters()
		{
			std::cout << "A: " << +regA << std::endl;
			std::cout << "X: " << +regX << std::endl;
			std::cout << "Y: " << +regY << std::endl;
		}

		template <class Type>
		void PrintMemory(int n)
		{
			std::cout << "Memory: [ ";
			for (int i = 0; i < n; i++) {
				std::cout << +(Type)memory[i] << " ";
			}
			std::cout << "]" << std::endl;
		}

		/// -------------------------------------------PRIVATE FUNCTIONS------------------------------------------------------- ///
	private:
		void Decode() // Fetches & decodes an instruction
		{
			std::cout << "Executing op-code: " << std::hex << "0x" << +memory[regPC] << std::endl;
			std::cout << "Offset: 0x" << regPC << std::dec << std::endl;

			switch (memory[regPC]) {
			case 0x0: { // BRK
				regPC += 2;
				stack.Push((regPC >> 8) & 0xFF); // Push PC_High
				stack.Push(regPC & 0xFF);        // Push PC_Low
				stack.Push(regStatus | (1 << Flag_B) | (1 << Flag_Unused));
				regStatus.Set(Flag_I);
				regPC = memory.Get16At(IRQVector);
				break;
			}
			case 0xA0: { // LDY Immediate
				regY = memory[regPC + 1];
				SetFlagsNZ(regY);
				regPC += 2;
				break;
			}
			case 0xA4: { // LDY Zeropage
				uint8 offset = memory[regPC + 1];
				regY = memory[offset];
				SetFlagsNZ(regY);
				regPC += 2;
				break;
			}
			case 0xB4: { // LDY Zeropage, X
				uint8 offset = memory[regPC + 1] + regX;
				regY = memory[offset];
				SetFlagsNZ(regY);
				regPC += 2;
				break;
			}
			case 0xAC: { // LDY Absolute
				uint16 offset = memory.Get16At(regPC + 1);
				regY = memory[offset];
				SetFlagsNZ(regY);
				regPC += 3;
				break;
			}
			case 0xBC: { // LDY Absolute, X
				uint16 offset = memory.Get16At(regPC + 1) + regX;
				regY = memory[offset];
				SetFlagsNZ(regY);
				regPC += 3;
				break;
			}

			case 0xA2: { // LDX Immediate
				regX = memory[regPC + 1];
				SetFlagsNZ(regX);
				regPC += 2;
				break;
			}
			case 0xA6: { // LDX Zeropage
				uint8 offset = memory[regPC + 1];
				regX = memory[offset];
				SetFlagsNZ(regX);
				regPC += 2;
				break;
			}
			case 0xB6: { // LDX Zeropage, Y
				uint8 offset = memory[regPC + 1] + regY;
				regX = memory[offset];
				SetFlagsNZ(regX);
				regPC += 2;
				break;
			}
			case 0xAE: { // LDX Absolute
				uint16 offset = memory.Get16At(regPC + 1);
				regX = memory[offset];
				SetFlagsNZ(regX);
				regPC += 3;
				break;
			}
			case 0xBE: { // LDX Absolute, Y
				uint16 offset = memory.Get16At(regPC + 1) + regY;
				regX = memory[offset];
				SetFlagsNZ(regX);
				regPC += 3;
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
				regA = memory[regPC + 1];
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0xA5: { // LDA Zeropage
				uint8 offset = memory[regPC + 1];
				regA = memory[offset];
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0xB5: { // LDA Zeropage, X
				uint8 offset = memory[regPC + 1] + regX;
				regA = memory[offset];
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0xAD: { // LDA Absolute
				uint16 offset = memory.Get16At(regPC + 1);
				regA = memory[offset];
				SetFlagsNZ(regA);
				regPC += 3;
				break;
			}
			case 0xBD: { // LDA Absolute, X
				uint16 offset = memory.Get16At(regPC + 1) + regX;
				regA = memory[offset];
				SetFlagsNZ(regA);
				regPC += 3;
				break;
			}
			case 0xB9: { // LDA Absolute, Y
				uint16 offset = memory.Get16At(regPC + 1) + regY;
				regA = memory[offset];
				SetFlagsNZ(regA);
				regPC += 3;
				break;
			}
			case 0xA1: { // LDA Indexed Indirect, X (Add first then fetch)
				uint8 offset = memory[regPC + 1] + regX;
				uint16 adr = memory.Get16At(offset);
				regA = memory[adr];
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0xB1: { // LDA Indirect Indexed, Y (Fetch first then add)
				uint8 offset = memory[regPC + 1];
				uint16 adr = memory.Get16At(offset) + regY;
				regA = memory[adr];
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0x69: { // ADC Immediate
				uint8 oper = memory[regPC + 1];
				OpADC(oper);
				regPC += 2;
				break;
			}
			case 0x65: { // ADC Zeropage
				uint8 offset = memory[regPC + 1];
				uint8 oper = memory[offset];
				OpADC(oper);
				regPC += 2;
				break;
			}
			case 0x75: { // ADC Zeropage, X
				uint8 offset = memory[regPC + 1] + regX;
				uint8 oper = memory[offset];
				OpADC(oper);
				regPC += 2;
				break;
			}
			case 0x6D: { // ADC Absolute
				uint16 offset = memory.Get16At(regPC + 1);
				uint8 oper = memory[offset];
				OpADC(oper);
				regPC += 3;
				break;
			}
			case 0x7D: { // ADC Absolute, X
				uint16 offset = memory.Get16At(regPC + 1) + regX;
				uint8 oper = memory[offset];
				OpADC(oper);
				regPC += 3;
				break;
			}
			case 0x79: { // ADC Absolute, Y
				uint16 offset = memory.Get16At(regPC + 1) + regY;
				uint8 oper = memory[offset];
				OpADC(oper);
				regPC += 3;
				break;
			}
			case 0x61: { // ADC Indexed Indirect, X (Add first then fetch)
				uint8 offset = memory[regPC + 1] + regX;
				uint16 adr = memory.Get16At(offset);
				uint8 oper = memory[adr];
				OpADC(oper);
				regPC += 2;
				break;
			}
			case 0x71: { // ADC Indirect Indexed, Y (Fetch first then add)
				uint8 offset = memory[regPC + 1];
				uint16 adr = memory.Get16At(offset) + regY;
				uint8 oper = memory[adr];
				OpADC(oper);
				regPC += 2;
				break;
			}
			case 0xE9: { // SBC Immediate
				uint8 oper = memory[regPC + 1];
				OpSBC(oper);
				regPC += 2;
				break;
			}
			case 0xE5: { // SBC Zeropage
				uint8 offset = memory[regPC + 1];
				uint8 oper = memory[offset];
				OpSBC(oper);
				regPC += 2;
				break;
			}
			case 0xF5: { // SBC Zeropage, X
				uint8 offset = memory[regPC + 1] + regX;
				uint8 oper = memory[offset];
				OpSBC(oper);
				regPC += 2;
				break;
			}
			case 0xED: { // SBC Absolute
				uint16 offset = memory.Get16At(regPC + 1);
				uint8 oper = memory[offset];
				OpSBC(oper);
				regPC += 3;
				break;
			}
			case 0xFD: { // SBC Absolute, X
				uint16 offset = memory.Get16At(regPC + 1) + regX;
				uint8 oper = memory[offset];
				OpSBC(oper);
				regPC += 3;
				break;
			}
			case 0xF9: { // SBC Absolute, Y
				uint16 offset = memory.Get16At(regPC + 1) + regY;
				uint8 oper = memory[offset];
				OpSBC(oper);
				regPC += 3;
				break;
			}
			case 0xE1: { // SBC Indexed Indirect, X (Add first then fetch)
				uint8 offset = memory[regPC + 1] + regX;
				uint16 adr = memory.Get16At(offset);
				uint8 oper = memory[adr];
				OpSBC(oper);
				regPC += 2;
				break;
			}
			case 0xF1: { // SBC Indirect Indexed, Y (Fetch first then add)
				uint8 offset = memory[regPC + 1];
				uint16 adr = memory.Get16At(offset) + regY;
				uint8 oper = memory[adr];
				OpSBC(oper);
				regPC += 2;
				break;
			}
			case 0x85: { // STA Zeropage
				uint8 offset = memory[regPC + 1];
				memory[offset] = regA;
				regPC += 2;
				break;
			}
			case 0x95: { // STA Zeropage, X
				uint8 offset = memory[regPC + 1] + regX;
				memory[offset] = regA;
				regPC += 2;
				break;
			}
			case 0x8D: { // STA Absolute
				uint16 adr = memory.Get16At(regPC + 1);
				memory[adr] = regA;
				regPC += 3;
				break;
			}
			case 0x9D: { // STA Absolute, X
				uint16 adr = memory.Get16At(regPC + 1) + regX;
				memory[adr] = regA;
				regPC += 3;
				break;
			}
			case 0x99: { // STA Absolute, Y
				uint16 adr = memory.Get16At(regPC + 1) + regY;
				memory[adr] = regA;
				regPC += 3;
				break;
			}
			case 0x81: { // STA Indexed Indirect, X (Add first then
				     // fetch)
				uint8 offset = memory[regPC + 1] + regX;
				uint16 adr = memory.Get16At(offset);
				memory[adr] = regA;
				regPC += 2;
				break;
			}
			case 0x91: { // STA Indirect Indexed, Y (Fetch first
				     // then add)
				uint8 offset = memory[regPC + 1];
				uint16 adr = memory.Get16At(offset) + regY;
				memory[adr] = regA;
				regPC += 2;
				break;
			}
			case 0x86: { // STX Zeropage
				uint8 offset = memory[regPC + 1];
				memory[offset] = regX;
				regPC += 2;
				break;
			}
			case 0x96: { // STX Zeropage, Y
				uint8 offset = memory[regPC + 1] + regY;
				memory[offset] = regX;
				regPC += 2;
				break;
			}
			case 0x8E: { // STX, Absolute
				uint16 adr = memory.Get16At(regPC + 1);
				memory[adr] = regX;
				regPC += 3;
				break;
			}
			case 0x84: { // STY Zeropage
				uint8 offset = memory[regPC + 1];
				memory[offset] = regY;
				regPC += 2;
				break;
			}
			case 0x94: { // STY Zeropage, X
				uint8 offset = memory[regPC + 1] + regX;
				memory[offset] = regY;
				regPC += 2;
				break;
			}
			case 0x8C: { // STY Absolute
				uint16 adr = memory.Get16At(regPC + 1);
				memory[adr] = regY;
				regPC += 3;
				break;
			}
			case 0x4C: { // JMP Absolute
				regPC = memory.Get16At(regPC + 1);
				break;
			}
			case 0x6C: { // JMP Indirect
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
				SetFlagsNZ(regA);
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
				int8 oper = memory[regPC + 1];
				regPC += regStatus[Flag_N] == 0 ? oper + 2 : 2;
				break;
			}
			case 0xF0: { // BEQ
				int8 oper = memory[regPC + 1];
				regPC += regStatus[Flag_Z] ? oper + 2 : 2;
				break;
			}
			case 0x90: { // BCC
				int8 oper = memory[regPC + 1];
				regPC += regStatus[Flag_C] == 0 ? oper + 2 : 2;
				break;
			}
			case 0xB0: { // BCS
				int8 oper = memory[regPC + 1];
				regPC += regStatus[Flag_C] ? oper + 2 : 2;
				break;
			}
			case 0x30: { // BMI
				int8 oper = memory[regPC + 1];
				regPC += regStatus[Flag_N] ? oper + 2 : 2;
				break;
			}
			case 0xD0: { // BNE
				int8 oper = memory[regPC + 1];
				regPC += regStatus[Flag_Z] == 0 ? oper + 2  : 2;
				break;
			}
			case 0x50: { // BVC
				int8 oper = memory[regPC + 1];
				regPC += regStatus[Flag_V] == 0 ? oper + 2 : 2;
				break;
			}
			case 0x70: { // BVS
				int8 oper = memory[regPC + 1];
				regPC += regStatus[Flag_V] ? oper + 2 : 2;
				break;
			}
			case 0x24: { // BIT Zeropage
				uint8 offset = memory[regPC + 1];
				uint8 oper = memory[offset];
				regStatus.Set(Flag_N, oper & Bit7);
				regStatus.Set(Flag_V, oper & Bit6);
				regStatus.Set(Flag_Z, (oper & regA) == 0);
				regPC += 2;
				break;
			}
			case 0x2C: { // BIT Absolute
				uint16 adr = memory.Get16At(regPC + 1);
				uint8 oper = memory[adr];
				regStatus.Set(Flag_N, oper & Bit7);
				regStatus.Set(Flag_V, oper & Bit6);
				regStatus.Set(Flag_Z, (oper & regA) == 0);
				regPC += 3;
				break;
			}
			case 0xCA: { // DEX, Decrement X by 1
				regX--;
				SetFlagsNZ(regX);
				regPC++;
				break;
			}
			case 0x88: { // DEY, Decrement Y by 1
				regY--;
				regStatus.Set(Flag_Z, regY == 0);
				regStatus.Set(Flag_N, (regY & Bit7) == Bit7);
				regPC++;
				break;
			}
			case 0xC6: { // DEC Zeropage
				uint8 offset = memory[regPC + 1];
				uint8 &oper = memory[offset];
				oper--;
				regStatus.Set(Flag_Z, oper == 0);
				regStatus.Set(Flag_N, oper & Bit7);
				regPC += 2;
				break;
			}
			case 0xD6: { // DEC Zeropage, X
				uint8 offset = memory[regPC + 1] + regX;
				uint8 &oper = memory[offset];
				oper--;
				regStatus.Set(Flag_Z, oper == 0);
				regStatus.Set(Flag_N, oper & Bit7);
				regPC += 2;
				break;
			}
			case 0xCE: { // DEC Absolute
				uint16 adr = memory.Get16At(regPC + 1);
				uint8 &oper = memory[adr];
				oper--;
				regStatus.Set(Flag_Z, oper == 0);
				regStatus.Set(Flag_N, oper & Bit7);
				regPC += 3;
				break;
			}
			case 0xDE: { // DEC Absolute, X
				uint16 adr = memory.Get16At(regPC + 1) + regX;
				uint8 &oper = memory[adr];
				oper--;
				regStatus.Set(Flag_Z, oper == 0);
				regStatus.Set(Flag_N, oper & Bit7);
				regPC += 3;
				break;
			}
			case 0x8A: { // TXA (Transfer X -> A)
				regA = regX;
				SetFlagsNZ(regA);
				regPC++;
				break;
			}
			case 0xAA: { // TAX (Transfer A -> X)
				regX = regA;
				SetFlagsNZ(regX);
				regPC++;
				break;
			}
			case 0xA8: { // TAY (Transfer A -> Y)
				regY = regA;
				SetFlagsNZ(regY);
				regPC++;
				break;
			}
			case 0xBA: { // TSX (Transfer SP -> X)
				regX = stack.GetSP();
				SetFlagsNZ(regX);
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
				SetFlagsNZ(regA);
				regPC++;
				break;
			}

			case 0x0A: { // ASL Accumulator (shift left)
				regStatus.Set(Flag_C, (regA & Bit7) == Bit7);
				regA <<= 1;
				SetFlagsNZ(regA);
				regPC++;
				break;
			}
			case 0x06: { // ASL Zeropage
				uint8 offset = memory[regPC + 1];
				uint8 &oper = memory[offset];
				regStatus.Set(Flag_C, oper & Bit7);
				oper <<= 1;
				SetFlagsNZ(oper);
				regPC += 2;
				break;
			}
			case 0x16: { // ASL Zeropage, X
				uint8 offset = memory[regPC + 1] + regX;
				uint8 &oper = memory[offset];
				regStatus.Set(Flag_C, oper & Bit7);
				oper <<= 1;
				SetFlagsNZ(oper);
				regPC += 2;
				break;
			}
			case 0x0E: { // ASL Absolute
				uint16 adr = memory.Get16At(regPC + 1);
				uint8 &oper = memory[adr];
				regStatus.Set(Flag_C, oper & Bit7);
				oper <<= 1;
				SetFlagsNZ(oper);
				regPC += 3;
				break;
			}
			case 0x1E: { // ASL Absolute, X
				uint16 adr = memory.Get16At(regPC + 1) + regX;
				uint8 &oper = memory[adr];
				regStatus.Set(Flag_C, oper & Bit7);
				oper <<= 1;
				SetFlagsNZ(oper);
				regPC += 3;
				break;
			}
			case 0x4A: { // LSR Accumulator (shift right)
				regStatus.Set(Flag_C, regA & Bit0);
				regA >>= 1;
				regStatus.Set(Flag_Z, regA == 0);
				regStatus.Clear(Flag_N);
				regPC++;
				break;
			}
			case 0x46: { // LSR Zeropage
				uint8 offset = memory[regPC + 1];
				uint8 &oper = memory[offset];
				regStatus.Set(Flag_C, oper & Bit0);
				oper >>= 1;
				regStatus.Set(Flag_Z, oper == 0);
				regStatus.Clear(Flag_N);
				regPC += 2;
				break;
			}
			case 0x56: { // LSR Zeropage, X
				uint8 offset = memory[regPC + 1] + regX;
				uint8 &oper = memory[offset];
				regStatus.Set(Flag_C, oper & Bit0);
				oper >>= 1;
				regStatus.Set(Flag_Z, oper == 0);
				regStatus.Clear(Flag_N);
				regPC += 2;
				break;
			}
			case 0x4E: { // LSR Absolute
				uint16 adr = memory.Get16At(regPC + 1);
				uint8 &oper = memory[adr];
				regStatus.Set(Flag_C, oper & Bit0);
				oper >>= 1;
				regStatus.Set(Flag_Z, oper == 0);
				regStatus.Clear(Flag_N);
				regPC += 3;
				break;
			}
			case 0x5E: { // LSR Absolute, X
				uint16 adr = memory.Get16At(regPC + 1) + regX;
				uint8 &oper = memory[adr];
				regStatus.Set(Flag_C, oper & Bit0);
				oper >>= 1;
				regStatus.Set(Flag_Z, oper == 0);
				regStatus.Clear(Flag_N);
				regPC += 3;
				break;
			}
			case 0x2A: { // ROL Accumulator (rotate left)
				uint16 temp = regA;                            // Holds carry in bit 8
				temp <<= 1;                                    // Shifts left one bit
				temp ^= (-regStatus[Flag_C] ^ temp) & Bit0;    // Changes bit 0 to whatever carry is
				regStatus.Set(Flag_C, temp & Bit8);            // Sets carry flag to whatever bit 8 is
				regA = temp & 0xFF;
				SetFlagsNZ(regA);
				regPC++;
				break;
			}
			case 0x26: { // ROL Zeropage
				uint8 offset = memory[regPC + 1];
				uint8 &oper = memory[offset];
				uint16 temp = oper;
				temp <<= 1;
				temp ^= (-regStatus[Flag_C] ^ temp) & Bit0;
				regStatus.Set(Flag_C, temp & Bit8);
				oper = temp & 0xFF;
				SetFlagsNZ(oper);
				regPC += 2;
				break;
			}
			case 0x36: { // ROL Zeropage, X
				uint8 offset = memory[regPC + 1] + regX;
				uint8 &oper = memory[offset];
				uint16 temp = oper;
				temp <<= 1;
				temp ^= (-regStatus[Flag_C] ^ temp) & Bit0;
				regStatus.Set(Flag_C, temp & Bit8);
				oper = temp & 0xFF;
				SetFlagsNZ(oper);
				regPC += 2;
				break;
			}
			case 0x2E: { // ROL Absolute
				uint16 adr = memory.Get16At(regPC + 1);
				uint8 &oper = memory[adr];
				uint16 temp = oper;
				temp <<= 1;
				temp ^= (-regStatus[Flag_C] ^ temp) & Bit0;
				regStatus.Set(Flag_C, temp & Bit8);
				oper = temp & 0xFF;
				SetFlagsNZ(oper);
				regPC += 3;
				break;
			}
			case 0x3E: { // ROL Absolute, X
				uint16 adr = memory.Get16At(regPC + 1) + regX;
				uint8 &oper = memory[adr];
				uint16 temp = oper;
				temp <<= 1;
				temp ^= (-regStatus[Flag_C] ^ temp) & Bit0;
				regStatus.Set(Flag_C, temp & Bit8);
				oper = temp & 0xFF;
				SetFlagsNZ(oper);
				regPC += 3;
				break;
			}
			case 0x6A: { // ROR Accumulator (rotate right)
				uint16 temp = regA;
				temp ^= (-regStatus[Flag_C] ^ temp) & Bit8;
				regStatus.Set(Flag_C, temp & Bit0);
				temp >>= 1;
				regA = temp & 0xFF;
				SetFlagsNZ(regA);
				regPC++;
				break;
			}
			case 0x66: { // ROR Zeropage
				uint8 offset = memory[regPC + 1];
				uint8 &oper = memory[offset];
				uint16 temp = oper;
				temp ^= (-regStatus[Flag_C] ^ temp) & Bit8;
				regStatus.Set(Flag_C, temp & Bit0);
				temp >>= 1;
				oper = temp & 0xFF;
				SetFlagsNZ(oper);
				regPC += 2;
				break;
			}
			case 0x76: { // ROR Zeropage, X
				uint8 offset = memory[regPC + 1] + regX;
				uint8 &oper = memory[offset];
				uint16 temp = oper;
				temp ^= (-regStatus[Flag_C] ^ temp) & Bit8;
				regStatus.Set(Flag_C, temp & Bit0);
				temp >>= 1;
				oper = temp & 0xFF;
				SetFlagsNZ(oper);
				regPC += 2;
				break;
			}
			case 0x6E: { // ROR Absolute
				uint16 adr = memory.Get16At(regPC + 1);
				uint8 &oper = memory[adr];
				uint16 temp = oper;
				temp ^= (-regStatus[Flag_C] ^ temp) & Bit8;
				regStatus.Set(Flag_C, temp & Bit0);
				temp >>= 1;
				oper = temp & 0xFF;
				SetFlagsNZ(oper);
				regPC += 3;
				break;
			}
			case 0x7E: { // ROR Absolute, X
				uint16 adr = memory.Get16At(regPC + 1) + regX;
				uint8 &oper = memory[adr];
				uint16 temp = oper;
				temp ^= (-regStatus[Flag_C] ^ temp) & Bit8;
				regStatus.Set(Flag_C, temp & Bit0);
				temp >>= 1;
				oper = temp & 0xFF;
				SetFlagsNZ(oper);
				regPC += 3;
				break;
			}
			case 0x29: { // AND Immediate
				uint8 oper = memory[regPC + 1];
				regA &= oper;
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0x25: { // AND Zeropage
				uint8 offset = memory[regPC + 1];
				regA &= memory[offset];
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0x35: { // AND Zeropage, X
				uint8 offset = memory[regPC + 1] + regX;
				regA &= memory[offset];
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0x2D: { // AND Absolute
				uint16 adr = memory.Get16At(regPC + 1);
				regA &= memory[adr];
				SetFlagsNZ(regA);
				regPC += 3;
				break;
			}
			case 0x3D: { // AND Absolute, X
				uint16 adr = memory.Get16At(regPC + 1) + regX;
				regA &= memory[adr];
				SetFlagsNZ(regA);
				regPC += 3;
				break;
			}
			case 0x39: { // AND Absolute, Y
				uint16 adr = memory.Get16At(regPC + 1) + regY;
				regA &= memory[adr];
				SetFlagsNZ(regA);
				regPC += 3;
				break;
			}
			case 0x21: { // AND Indexed Indirect, X (Add first then fetch)
				uint8 offset = memory[regPC + 1] + regX;
				uint16 adr = memory.Get16At(offset);
				regA &= memory[adr];
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0x31: { // AND Indirect Indexed, Y (Fetch first then add)
				uint8 offset = memory[regPC + 1];
				uint16 adr = memory.Get16At(offset) + regY;
				regA &= memory[adr];
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0xC9: { // CMP Immediate
				uint8 oper = memory[regPC + 1];
				OpCMP(oper, regA);
				regPC += 2;
				break;
			}
			case 0xC5: { // CMP Zeropage
				uint8 offset = memory[regPC + 1];
				uint8 oper = memory[offset];
				OpCMP(oper, regA);
				regPC += 2;
				break;
			}
			case 0xD5: { // CMP Zeropage, X
				uint8 offset = memory[regPC + 1] + regX;
				uint8 oper = memory[offset];
				OpCMP(oper, regA);
				regPC += 2;
				break;
			}
			case 0xCD: { // CMP Absolute
				uint16 adr = memory.Get16At(regPC + 1);
				uint8 oper = memory[adr];
				OpCMP(oper, regA);
				regPC += 3;
				break;
			}
			case 0xDD: { // CMP Absolute, X
				uint16 adr = memory.Get16At(regPC + 1) + regX;
				uint8 oper = memory[adr];
				OpCMP(oper, regA);
				regPC += 3;
				break;
			}
			case 0xD9: { // CMP Absolute, Y
				uint16 adr = memory.Get16At(regPC + 1) + regY;
				uint8 oper = memory[adr];
				OpCMP(oper, regA);
				regPC += 3;
				break;
			}
			case 0xC1: { // CMP Indexed Indirect, X (Add first then fetch)
				uint8 offset = memory[regPC + 1] + regX;
				uint16 adr = memory.Get16At(offset);
				uint8 oper = memory[adr];
				OpCMP(oper, regA);
				regPC += 2;
				break;
			}
			case 0xD1: { // CMP Indirect Indexed, Y (Fetch first then add)
				uint8 offset = memory[regPC + 1];
				uint16 adr = memory.Get16At(offset) + regY;
				uint8 oper = memory[adr];
				OpCMP(oper, regA);
				regPC += 2;
				break;
			}
			case 0xE0: { // CPX Immediate
				uint8 oper = memory[regPC + 1];
				OpCMP(oper, regX);
				regPC += 2;
				break;
			}
			case 0xE4: { // CPX Zeropage
				uint8 offset = memory[regPC + 1];
				uint8 oper = memory[offset];
				OpCMP(oper, regX);
				regPC += 2;
				break;
			}
			case 0xEC: { // CPX Absolute
				uint16 adr = memory.Get16At(regPC + 1);
				uint8 oper = memory[adr];
				OpCMP(oper, regX);
				regPC += 3;
				break;
			}
			case 0xC0: { // CPY Immediate
				uint8 oper = memory[regPC + 1];
				OpCMP(oper, regY);
				regPC += 2;
				break;
			}
			case 0xC4: { // CPY Zeropage
				uint8 offset = memory[regPC + 1];
				uint8 oper = memory[offset];
				OpCMP(oper, regY);
				regPC += 2;
				break;
			}
			case 0xCC: { // CPY Absolute
				uint16 adr = memory.Get16At(regPC + 1);
				uint8 oper = memory[adr];
				OpCMP(oper, regY);
				regPC += 3;
				break;
			}
			case 0x09: { // ORA Immediate
				uint8 oper = memory[regPC + 1];
				regA |= oper;
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0x05: { // ORA Zeropage
				uint8 offset = memory[regPC + 1];
				regA |= memory[offset];
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0x15: { // ORA Zeropage, X
				uint8 offset = memory[regPC + 1] + regX;
				regA |= memory[offset];
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0x0D: { // ORA Absolute
				uint16 adr = memory.Get16At(regPC + 1);
				regA |= memory[adr];
				SetFlagsNZ(regA);
				regPC += 3;
				break;
			}
			case 0x1D: { // ORA Absolute, X
				uint16 adr = memory.Get16At(regPC + 1) + regX;
				regA |= memory[adr];
				SetFlagsNZ(regA);
				regPC += 3;
				break;
			}
			case 0x19: { // ORA Absolute, Y
				uint16 adr = memory.Get16At(regPC + 1) + regY;
				regA |= memory[adr];
				SetFlagsNZ(regA);
				regPC += 3;
				break;
			}
			case 0x01: { // ORA Indexed Indirect, X (Add first then fetch)
				uint8 offset = memory[regPC + 1] + regX;
				uint16 adr = memory.Get16At(offset);
				regA |= memory[adr];
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0x11: { // ORA Indirect Indexed, Y (Fetch first then add)
				uint8 offset = memory[regPC + 1];
				uint16 adr = memory.Get16At(offset) + regY;
				regA |= memory[adr];
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0x49: { // EOR Immediate
				uint8 oper = memory[regPC + 1];
				regA ^= oper;
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0x45: { // EOR Zeropage
				uint8 offset = memory[regPC + 1];
				regA ^= memory[offset];
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0x55: { // EOR Zeropage, X
				uint8 offset = memory[regPC + 1] + regX;
				regA ^= memory[offset];
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0x4D: { // EOR Absolute
				uint16 adr = memory.Get16At(regPC + 1);
				regA ^= memory[adr];
				SetFlagsNZ(regA);
				regPC += 3;
				break;
			}
			case 0x5D: { // EOR Absolute, X
				uint16 adr = memory.Get16At(regPC + 1) + regX;
				regA ^= memory[adr];
				SetFlagsNZ(regA);
				regPC += 3;
				break;
			}
			case 0x59: { // EOR Absolute, Y
				uint16 adr = memory.Get16At(regPC + 1) + regY;
				regA ^= memory[adr];
				SetFlagsNZ(regA);
				regPC += 3;
				break;
			}
			case 0x41: { // EOR Indexed Indirect, X (Add first then
				     // fetch)
				uint8 offset = memory[regPC + 1] + regX;
				uint16 adr = memory.Get16At(offset);
				regA ^= memory[adr];
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0x51: { // EOR Indirect Indexed, Y (Fetch first
				     // then add)
				uint8 offset = memory[regPC + 1];
				uint16 adr = memory.Get16At(offset) + regY;
				regA ^= memory[adr];
				SetFlagsNZ(regA);
				regPC += 2;
				break;
			}
			case 0xE6: { // INC Zeropage
				uint8 offset = memory[regPC + 1];
				memory[offset]++;
				SetFlagsNZ(memory[offset]);
				regPC += 2;
				break;
			}
			case 0xF6: { // INC Zeropage, X
				uint8 offset = memory[regPC + 1] + regX;
				memory[offset]++;
				SetFlagsNZ(memory[offset]);
				regPC += 2;
				break;
			}
			case 0xEE: { // INC Absolute
				uint16 adr = memory.Get16At(regPC + 1);
				memory[adr]++;
				SetFlagsNZ(memory[adr]);
				regPC += 3;
				break;
			}
			case 0xFE: { // INC Absolute, X
				uint16 adr = memory.Get16At(regPC + 1) + regX;
				memory[adr]++;
				SetFlagsNZ(memory[adr]);
				regPC += 3;
				break;
			}
			case 0xE8: { // INX
				regX++;
				SetFlagsNZ(regX);
				regPC++;
				break;
			}
			case 0xC8: { // INY
				regY++;
				SetFlagsNZ(regY);
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

		void OpADC(uint8 oper)
		{
			uint result = regA + oper + regStatus[Flag_C];
			bool overflow = !((regA ^ oper) & 0x80) && ((regA ^ result) & 0x80);
			regStatus.Set(Flag_Z, (result & 0xFF) == 0);
			regStatus.Set(Flag_C, result > 0xFF);	           
			regStatus.Set(Flag_V, overflow);                // If the signs of both operands != the sign of the result, set overflow
			regStatus.Set(Flag_N, result & Bit7);
			regA = result & 0xFF;
		}

		void OpSBC(uint8 oper)
		{
			uint result = regA - oper - !regStatus[Flag_C];
			bool overflow = ((regA ^ result) & Bit7) && ((regA ^ oper) & Bit7);
			regStatus.Set(Flag_Z, (result & 0xFF) == 0);
			regStatus.Set(Flag_C, result < Bit8);
			regStatus.Set(Flag_V, overflow);                                      
			regStatus.Set(Flag_N, result & Bit7);
			regA = result & 0xFF;
		}

		void OpCMP(uint8 oper, uint8 reg)
		{
			uint result = reg - oper;
			regStatus.Set(Flag_Z, reg == oper);
			regStatus.Set(Flag_N, result & Bit7);
			regStatus.Set(Flag_C, result < Bit8);
		}

		void SetFlagsNZ(uint8 reg)
		{
			regStatus.Set(Flag_N, reg & Bit7);   // If 7th bit is 1, set negative
			regStatus.Set(Flag_Z, reg == 0);  	 // If register is 0, set zero
		}
	};

} // namespace nemu