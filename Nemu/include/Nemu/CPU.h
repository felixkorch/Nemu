#pragma once
#include "Nemu/Stack.h"
#include "Nemu/NESMemory.h"
#include "Nemu/Mapper/NROM256Mapper.h"
#include <cstdint>
#include <vector>
#include <array>
#include <iostream>

namespace nemu
{
	typedef std::uint8_t uint8;
	typedef std::uint16_t uint16;
	typedef std::int8_t int8;
	typedef unsigned int uint;
#define AsUInt16(x) *(uint16 *)&memory[x]

	template <class Memory>
	class CPU {
	    private:
		constexpr static std::size_t STACK_SIZE = 256;

		uint8 reg_X;
		uint8 reg_Y;
		uint8 reg_A;

		uint16 reg_PC;
		std::array<uint8, 8> reg_S;

		using Storage = std::vector<uint8>;
		Memory memory;
		Stack<uint8, STACK_SIZE> stack;
		bool running;

		// Status bits
		constexpr static uint8 B_C = 0;
		constexpr static uint8 B_Z = 1;
		constexpr static uint8 B_I = 2;
		constexpr static uint8 B_BCD = 3; // Disabled (decimal mode)?
		constexpr static uint8 B_BRK = 4; // TODO: Not used
		constexpr static uint8 B_O = 6;
		constexpr static uint8 B_N = 7;

		constexpr static uint8 B_0 = (1 << 0);
		constexpr static uint8 B_1 = (1 << 1);
		constexpr static uint8 B_2 = (1 << 2);
		constexpr static uint8 B_3 = (1 << 3);
		constexpr static uint8 B_4 = (1 << 4);
		constexpr static uint8 B_5 = (1 << 5);
		constexpr static uint8 B_6 = (1 << 6);
		constexpr static uint8 B_7 = (1 << 7);
		constexpr static uint16 B_8 = (1 << 8);

	    public:
		CPU()
			: reg_X(0), reg_Y(0), reg_A(0), reg_PC(0), reg_S{},
			  stack(&memory[0] + 0x1FF), // Stack range 0x100 -> 0x1FF
			  running(false)
		{}

		uint8 ToBitMask(std::array<uint8, 8> array)
		{
			uint8 mask = 0;
			for (uint i = 0; i < 8; i++)
				mask |= (array[i] << i);
			return mask;
		}

		void FromBitMask(uint8 mask, std::array<uint8, 8> &to)
		{
			for (uint i = 0; i < 8; i++)
				to[i] = ((1 << i) & mask) == (1 << i);
		}

		bool Decode()
		{
			std::cout << "Executing op-code: " << std::hex << "0x"
				  << +memory[reg_PC] << std::dec << std::endl;
			switch (memory[reg_PC]) {
			case 0x0: { // BRK
				reg_S[B_I] = 1;
				stack.Push(reg_PC + 2);
				stack.Push(ToBitMask(reg_S));
				reg_PC++;
				return true;
			}
			case 0xA0: { // LDY Immediate
				reg_Y = memory[reg_PC + 1];
				SetFlags_NZ(reg_Y);
				reg_PC += 2;
				return true;
			}
			case 0xA4: { // LDY Zeropage
				uint8 offset = memory[reg_PC + 1];
				reg_Y = memory[offset];
				SetFlags_NZ(reg_Y);
				reg_PC += 2;
				return true;
			}
			case 0xB4: { // LDY Zeropage, X
				uint8 offset = memory[reg_PC + 1] + reg_X;
				reg_Y = memory[offset];
				SetFlags_NZ(reg_Y);
				reg_PC += 2;
				return true;
			}
			case 0xAC: { // LDY Absolute
				uint16 offset = AsUInt16(reg_PC + 1);
				reg_Y = memory[offset];
				SetFlags_NZ(reg_Y);
				reg_PC += 3;
				return true;
			}
			case 0xBC: { // LDY Absolute, X
				uint16 offset = AsUInt16(reg_PC + 1) + reg_X;
				reg_Y = memory[offset];
				SetFlags_NZ(reg_Y);
				reg_PC += 3;
				return true;
			}

			case 0xA2: { // LDX Immediate
				reg_X = memory[reg_PC + 1];
				SetFlags_NZ(reg_X);
				reg_PC += 2;
				return true;
			}
			case 0xA6: { // LDX Zeropage
				uint8 offset = memory[reg_PC + 1];
				reg_X = memory[offset];
				SetFlags_NZ(reg_X);
				reg_PC += 2;
				return true;
			}
			case 0xB6: { // LDX Zeropage, Y
				uint8 offset = memory[reg_PC + 1] + reg_Y;
				reg_X = memory[offset];
				SetFlags_NZ(reg_X);
				reg_PC += 2;
				return true;
			}
			case 0xAE: { // LDX Absolute
				uint16 offset = AsUInt16(reg_PC + 1);
				reg_X = memory[offset];
				SetFlags_NZ(reg_X);
				reg_PC += 3;
				return true;
			}
			case 0xBE: { // LDX Absolute, Y
				uint16 offset = AsUInt16(reg_PC + 1) + reg_Y;
				reg_X = memory[offset];
				SetFlags_NZ(reg_X);
				reg_PC += 3;
				return true;
			}
			case 0xEA: { // NOP
				reg_PC++;
				return true;
			}
			case 0x18: { // CLC
				reg_S[B_C] = 0;
				reg_PC++;
				return true;
			}
			case 0xD8: { // CLD
				reg_S[B_BCD] = 0;
				reg_PC++;
				return true;
			}
			case 0x58: { // CLI
				reg_S[B_I] = 0;
				reg_PC++;
				return true;
			}
			case 0xB8: { // CLV
				reg_S[B_C] = 0;
				reg_PC++;
				return true;
			}
			case 0x38: { // SEC
				reg_S[B_C] = 1;
				reg_PC++;
				return true;
			}
			case 0xF8: { // SED
				reg_S[B_BCD] = 1;
				reg_PC++;
				return true;
			}
			case 0x78: { // SEI
				reg_S[B_I] = 1;
				reg_PC++;
				return true;
			}
			case 0xA9: { // LDA Immediate
				reg_A = memory[reg_PC + 1];
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0xA5: { // LDA Zeropage
				uint8 offset = memory[reg_PC + 1];
				reg_A = memory[offset];
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0xB5: { // LDA Zeropage, X
				uint8 offset = memory[reg_PC + 1] + reg_X;
				reg_A = memory[offset];
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0xAD: { // LDA Absolute
				uint16 offset = AsUInt16(reg_PC + 1);
				reg_A = memory[offset];
				SetFlags_NZ(reg_A);
				reg_PC += 3;
				return true;
			}
			case 0xBD: { // LDA Absolute, X
				uint16 offset = AsUInt16(reg_PC + 1) + reg_X;
				reg_A = memory[offset];
				SetFlags_NZ(reg_A);
				reg_PC += 3;
				return true;
			}
			case 0xB9: { // LDA Absolute, Y
				uint16 offset = AsUInt16(reg_PC + 1) + reg_Y;
				reg_A = memory[offset];
				SetFlags_NZ(reg_A);
				reg_PC += 3;
				return true;
			}
			case 0xA1: { // LDA Indexed Indirect, X (Add first then
				     // fetch)
				uint8 offset = memory[reg_PC + 1] + reg_X;
				uint16 adr = AsUInt16(offset);
				reg_A = memory[adr];
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0xB1: { // LDA Indirect Indexed, Y (Fetch first
				     // then add)
				uint8 offset = memory[reg_PC + 1];
				uint16 adr = AsUInt16(offset) + reg_Y;
				reg_A = memory[adr];
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0x69: { // ADC Immediate
				uint8 oper = memory[reg_PC + 1];
				OP_ADC(oper);
				reg_PC += 2;
				return true;
			}
			case 0x65: { // ADC Zeropage
				uint8 offset = memory[reg_PC + 1];
				uint8 oper = memory[offset];
				OP_ADC(oper);
				reg_PC += 2;
				return true;
			}
			case 0x75: { // ADC Zeropage, X
				uint8 offset = memory[reg_PC + 1] + reg_X;
				uint8 oper = memory[offset];
				OP_ADC(oper);
				reg_PC += 2;
				return true;
			}
			case 0x6D: { // ADC Absolute
				uint16 offset = AsUInt16(reg_PC + 1);
				uint8 oper = memory[offset];
				OP_ADC(oper);
				reg_PC += 3;
				return true;
			}
			case 0x7D: { // ADC Absolute, X
				uint16 offset = AsUInt16(reg_PC + 1) + reg_X;
				uint8 oper = memory[offset];
				OP_ADC(oper);
				reg_PC += 3;
				return true;
			}
			case 0x79: { // ADC Absolute, Y
				uint16 offset = AsUInt16(reg_PC + 1) + reg_Y;
				uint8 oper = memory[offset];
				OP_ADC(oper);
				reg_PC += 3;
				return true;
			}
			case 0x61: { // ADC Indexed Indirect, X (Add first then
				     // fetch)
				uint8 offset = memory[reg_PC + 1] + reg_X;
				uint16 adr = AsUInt16(offset);
				uint8 oper = memory[adr];
				OP_ADC(oper);
				reg_PC += 2;
				return true;
			}
			case 0x71: { // ADC Indirect Indexed, Y (Fetch first
				     // then add)
				uint8 offset = memory[reg_PC + 1];
				uint16 adr = AsUInt16(offset) + reg_Y;
				uint8 oper = memory[adr];
				OP_ADC(oper);
				reg_PC += 2;
				return true;
			}
			case 0xE9: { // SBC Immediate
				uint8 oper = memory[reg_PC + 1];
				OP_SBC(oper);
				reg_PC += 2;
				return true;
			}
			case 0xE5: { // SBC Zeropage
				uint8 offset = memory[reg_PC + 1];
				uint8 oper = memory[offset];
				OP_SBC(oper);
				reg_PC += 2;
				return true;
			}
			case 0xF5: { // SBC Zeropage, X
				uint8 offset = memory[reg_PC + 1] + reg_X;
				uint8 oper = memory[offset];
				OP_SBC(oper);
				reg_PC += 2;
				return true;
			}
			case 0xED: { // SBC Absolute
				uint16 offset = AsUInt16(reg_PC + 1);
				uint8 oper = memory[offset];
				OP_SBC(oper);
				reg_PC += 3;
				return true;
			}
			case 0xFD: { // SBC Absolute, X
				uint16 offset = AsUInt16(reg_PC + 1) + reg_X;
				uint8 oper = memory[offset];
				OP_SBC(oper);
				reg_PC += 3;
				return true;
			}
			case 0xF9: { // SBC Absolute, Y
				uint16 offset = AsUInt16(reg_PC + 1) + reg_Y;
				uint8 oper = memory[offset];
				OP_SBC(oper);
				reg_PC += 3;
				return true;
			}
			case 0xE1: { // SBC Indexed Indirect, X (Add first then
				     // fetch)
				uint8 offset = memory[reg_PC + 1] + reg_X;
				uint16 adr = AsUInt16(offset);
				uint8 oper = memory[adr];
				OP_SBC(oper);
				reg_PC += 2;
				return true;
			}
			case 0xF1: { // SBC Indirect Indexed, Y (Fetch first
				     // then add)
				uint8 offset = memory[reg_PC + 1];
				uint16 adr = AsUInt16(offset) + reg_Y;
				uint8 oper = memory[adr];
				OP_SBC(oper);
				reg_PC += 2;
				return true;
			}
			case 0x85: { // STA Zeropage
				uint8 offset = memory[reg_PC + 1];
				memory[offset] = reg_A;
				reg_PC += 2;
				return true;
			}
			case 0x95: { // STA Zeropage, X
				uint8 offset = memory[reg_PC + 1] + reg_X;
				memory[offset] = reg_A;
				reg_PC += 2;
				return true;
			}
			case 0x8D: { // STA Absolute
				uint16 adr = AsUInt16(reg_PC + 1);
				memory[adr] = reg_A;
				reg_PC += 3;
				return true;
			}
			case 0x9D: { // STA Absolute, X
				uint16 adr = AsUInt16(reg_PC + 1) + reg_X;
				memory[adr] = reg_A;
				reg_PC += 3;
				return true;
			}
			case 0x99: { // STA Absolute, Y
				uint16 adr = AsUInt16(reg_PC + 1) + reg_Y;
				memory[adr] = reg_A;
				reg_PC += 3;
				return true;
			}
			case 0x81: { // STA Indexed Indirect, X (Add first then
				     // fetch)
				uint8 offset = memory[reg_PC + 1] + reg_X;
				uint16 adr = AsUInt16(offset);
				memory[adr] = reg_A;
				reg_PC += 2;
				return true;
			}
			case 0x91: { // STA Indirect Indexed, Y (Fetch first
				     // then add)
				uint8 offset = memory[reg_PC + 1];
				uint16 adr = AsUInt16(offset) + reg_Y;
				memory[adr] = reg_A;
				reg_PC += 2;
				return true;
			}
			case 0x86: { // STX Zeropage
				uint8 offset = memory[reg_PC + 1];
				memory[offset] = reg_X;
				reg_PC += 2;
				return true;
			}
			case 0x96: { // STX Zeropage, Y
				uint8 offset = memory[reg_PC + 1] + reg_Y;
				memory[offset] = reg_X;
				reg_PC += 2;
				return true;
			}
			case 0x8E: { // STX, Absolute
				uint16 adr = AsUInt16(reg_PC + 1);
				memory[adr] = reg_X;
				reg_PC += 3;
				return true;
			}
			case 0x84: { // STY Zeropage
				uint8 offset = memory[reg_PC + 1];
				memory[offset] = reg_Y;
				reg_PC += 2;
				return true;
			}
			case 0x94: { // STY Zeropage, X
				uint8 offset = memory[reg_PC + 1] + reg_X;
				memory[offset] = reg_Y;
				reg_PC += 2;
				return true;
			}
			case 0x8C: { // STY Absolute
				uint16 adr = AsUInt16(reg_PC + 1);
				memory[adr] = reg_Y;
				reg_PC += 3;
				return true;
			}
			case 0x4C: { // JMP Absolute
				reg_PC = AsUInt16(reg_PC + 1);
				return true;
			}
			case 0x6C: { // JMP Indirect
				uint16 offset = AsUInt16(reg_PC + 1);
				reg_PC = memory[offset];
				return true;
			}
			case 0x20: { // JSR
				uint16 offset = AsUInt16(reg_PC + 1);
				uint16 jmp_adr = reg_PC + 2;
				stack.Push((jmp_adr >> 8));
				stack.Push(jmp_adr);
				reg_PC = offset;
				return true;
			}
			case 0x48: { // PHA
				stack.Push(reg_A);
				reg_PC++;
				return true;
			}
			case 0x08: { // PHP
				stack.Push(ToBitMask(reg_S));
				reg_PC++;
				return true;
			}
			case 0x68: { // PLA
				reg_A = stack.Pop();
				SetFlags_NZ(reg_A);
				reg_PC++;
				return true;
			}
			case 0x28: { // PLP
				FromBitMask(stack.Pop(), reg_S);
				reg_PC++;
				return true;
			}
			case 0x40: { // RTI
				FromBitMask(stack.Pop(), reg_S);
				reg_PC = stack.Pop();
				return true;
			}
			case 0x60: { // RTS
				uint16 reg_PC_low = stack.Pop();
				uint16 reg_PC_high = stack.Pop();
				uint16 return_adr =
					(reg_PC_high << 8) | reg_PC_low;
				reg_PC = return_adr + 1;
				return true;
			}
			case 0xF0: { // BEQ
				int8 oper = memory[reg_PC + 1];
				reg_PC += reg_S[B_Z] ? oper : 2;
				return true;
			}
			case 0x90: { // BCC
				int8 oper = memory[reg_PC + 1];
				reg_PC += reg_S[B_C] == 0 ? oper : 2;
				return true;
			}
			case 0xB0: { // BCS
				int8 oper = memory[reg_PC + 1];
				reg_PC += reg_S[B_C] ? oper : 2;
				return true;
			}
			case 0x30: { // BMI
				int8 oper = memory[reg_PC + 1];
				reg_PC += reg_S[B_N] ? oper : 2;
				return true;
			}
			case 0xD0: { // BNE
				int8 oper = memory[reg_PC + 1];
				reg_PC += reg_S[B_Z] == 0 ? oper : 2;
				return true;
			}
			case 0x50: { // BVC
				int8 oper = memory[reg_PC + 1];
				reg_PC += reg_S[B_O] == 0 ? oper : 2;
				return true;
			}
			case 0x70: { // BVS
				int8 oper = memory[reg_PC + 1];
				reg_PC += reg_S[B_O] ? oper : 2;
				return true;
			}
			case 0x24: { // BIT Zeropage
				uint8 offset = memory[reg_PC + 1];
				uint8 oper = memory[offset];
				reg_S[B_N] = (oper & B_7) == B_7;
				reg_S[B_O] = (oper & B_6) == B_6;
				reg_S[B_Z] = oper & reg_A;
				reg_PC += 2;
				return true;
			}
			case 0x2C: { // BIT Absolute
				uint16 adr = AsUInt16(reg_PC + 1);
				uint8 oper = memory[adr];
				reg_S[B_N] = (oper & B_7) == B_7;
				reg_S[B_O] = (oper & B_6) == B_6;
				reg_S[B_Z] = oper & reg_A;
				reg_PC += 3;
				return true;
			}
			case 0xCA: { // DEX, Decrement X by 1
				reg_X--;
				reg_S[B_Z] = reg_X == 0 ? 1 : 0;
				reg_S[B_N] = (reg_X & B_7) == B_7 ? 1 : 0;
				reg_PC++;
				return true;
			}
			case 0x88: { // DEY, Decrement Y by 1
				reg_Y--;
				reg_S[B_Z] = reg_Y == 0 ? 1 : 0;
				reg_S[B_N] = (reg_Y & B_7) == B_7 ? 1 : 0;
				reg_PC++;
				return true;
			}
			case 0xC6: { // DEC Zeropage
				uint8 offset = memory[reg_PC + 1];
				uint8 &oper = memory[offset];
				oper--;
				reg_S[B_Z] = oper == 0 ? 1 : 0;
				reg_S[B_N] = (oper & B_7) == B_7 ? 1 : 0;
				reg_PC += 2;
				return true;
			}
			case 0xD6: { // DEC Zeropage, X
				uint8 offset = memory[reg_PC + 1] + reg_X;
				uint8 &oper = memory[offset];
				oper--;
				reg_S[B_Z] = oper == 0 ? 1 : 0;
				reg_S[B_N] = (oper & B_7) == B_7 ? 1 : 0;
				reg_PC += 2;
				return true;
			}
			case 0xCE: { // DEC Absolute
				uint16 adr = AsUInt16(reg_PC + 1);
				uint8 &oper = memory[adr];
				oper--;
				reg_S[B_Z] = oper == 0 ? 1 : 0;
				reg_S[B_N] = (oper & B_7) == B_7 ? 1 : 0;
				reg_PC += 3;
				return true;
			}
			case 0xDE: { // DEC Absolute, X
				uint16 adr = AsUInt16(reg_PC + 1) + reg_X;
				uint8 &oper = memory[adr];
				oper--;
				reg_S[B_Z] = oper == 0 ? 1 : 0;
				reg_S[B_N] = (oper & B_7) == B_7 ? 1 : 0;
				reg_PC += 3;
				return true;
			}
			case 0x8A: { // TXA (Transfer X -> A)
				reg_A = reg_X;
				reg_PC++;
				return true;
			}
			case 0xAA: { // TAX (Transfer A -> X)
				reg_X = reg_A;
				reg_PC++;
				return true;
			}
			case 0xA8: { // TAY (Transfer A -> Y)
				reg_Y = reg_A;
				reg_PC++;
				return true;
			}
			case 0xBA: { // TSX (Transfer SP -> X)
				reg_X = stack.GetSP();
				reg_PC++;
				return true;
			}
			case 0x9A: { // TXS (Transfer X -> SP)
				stack.SetSP(reg_X);
				reg_PC++;
				return true;
			}
			case 0x98: { // TYA (Transfer Y -> A)
				reg_A = reg_Y;
				reg_PC++;
				return true;
			}
			case 0x10: { // BPL, Branch on result plus
				int8 oper = memory[reg_PC + 1];
				reg_PC += (reg_S[B_N] == 0) ? oper : 1;
				return true;
			}
			case 0x0A: { // ASL Accumulator (shift left)
				reg_S[B_C] = (reg_A & B_7) == B_7;
				reg_A <<= 1;
				SetFlags_NZ(reg_A);
				reg_PC++;
				return true;
			}
			case 0x06: { // ASL Zeropage
				uint8 offset = memory[reg_PC + 1];
				uint8 &oper = memory[offset];
				reg_S[B_C] = (oper & B_7) == B_7;
				oper <<= 1;
				SetFlags_NZ(oper);
				reg_PC += 2;
				return true;
			}
			case 0x16: { // ASL Zeropage, X
				uint8 offset = memory[reg_PC + 1] + reg_X;
				uint8 &oper = memory[offset];
				reg_S[B_C] = (oper & B_7) == B_7;
				oper <<= 1;
				SetFlags_NZ(oper);
				reg_PC += 2;
				return true;
			}
			case 0x0E: { // ASL Absolute
				uint16 adr = AsUInt16(reg_PC + 1);
				uint8 &oper = memory[adr];
				reg_S[B_C] = (oper & B_7) == B_7;
				oper <<= 1;
				SetFlags_NZ(oper);
				reg_PC += 3;
				return true;
			}
			case 0x1E: { // ASL Absolute, X
				uint16 adr = AsUInt16(reg_PC + 1) + reg_X;
				uint8 &oper = memory[adr];
				reg_S[B_C] = (oper & B_7) == B_7;
				oper <<= 1;
				SetFlags_NZ(oper);
				reg_PC += 3;
				return true;
			}
			case 0x4A: { // LSR Accumulator (shift right)
				reg_S[B_C] = (reg_A & B_0) == B_0;
				reg_A >>= 1;
				reg_S[B_Z] = reg_A == 0;
				reg_PC++;
				return true;
			}
			case 0x46: { // LSR Zeropage
				uint8 offset = memory[reg_PC + 1];
				uint8 &oper = memory[offset];
				reg_S[B_C] = (oper & B_0) == B_0;
				oper >>= 1;
				reg_S[B_Z] = oper == 0;
				reg_PC += 2;
				return true;
			}
			case 0x56: { // LSR Zeropage, X
				uint8 offset = memory[reg_PC + 1] + reg_X;
				uint8 &oper = memory[offset];
				reg_S[B_C] = (oper & B_0) == B_0;
				oper >>= 1;
				reg_S[B_Z] = oper == 0;
				reg_PC += 2;
				return true;
			}
			case 0x4E: { // LSR Absolute
				uint16 adr = AsUInt16(reg_PC + 1);
				uint8 &oper = memory[adr];
				reg_S[B_C] = (oper & B_0) == B_0;
				oper >>= 1;
				reg_S[B_Z] = oper == 0;
				reg_PC += 3;
				return true;
			}
			case 0x5E: { // LSR Absolute, X
				uint16 adr = AsUInt16(reg_PC + 1) + reg_X;
				uint8 &oper = memory[adr];
				reg_S[B_C] = (oper & B_0) == B_0;
				oper >>= 1;
				reg_S[B_Z] = oper == 0;
				reg_PC += 3;
				return true;
			}
			case 0x2A: { // ROL Accumulator (rotate left)
				reg_S[B_C] = (reg_A & B_7) == B_7; // Sets carry
								   // flag to
								   // whatever
								   // bit 7 is
				reg_A <<= 1; // Shifts left one bit
				reg_A ^= (-reg_S[B_C] ^ reg_A) & B_0; // Changes
								      // bit 0
								      // to
								      // whatever
								      // carry
								      // is
				SetFlags_NZ(reg_A);
				reg_PC++;
				return true;
			}
			case 0x26: { // ROL Zeropage
				uint8 offset = memory[reg_PC + 1];
				uint8 &oper = memory[offset];
				reg_S[B_C] = (oper & B_7) == B_7;
				oper <<= 1;
				oper ^= (-reg_S[B_C] ^ oper) & B_0;
				SetFlags_NZ(oper);
				reg_PC += 2;
				return true;
			}
			case 0x36: { // ROL Zeropage, X
				uint8 offset = memory[reg_PC + 1] + reg_X;
				uint8 &oper = memory[offset];
				reg_S[B_C] = (oper & B_7) == B_7;
				oper <<= 1;
				oper ^= (-reg_S[B_C] ^ oper) & B_0;
				SetFlags_NZ(oper);
				reg_PC += 2;
				return true;
			}
			case 0x2E: { // ROL Absolute
				uint16 adr = AsUInt16(reg_PC + 1);
				uint8 &oper = memory[adr];
				reg_S[B_C] = (oper & B_7) == B_7;
				oper <<= 1;
				oper ^= (-reg_S[B_C] ^ oper) & B_0;
				SetFlags_NZ(oper);
				reg_PC += 3;
				return true;
			}
			case 0x3E: { // ROL Absolute, X
				uint16 adr = AsUInt16(reg_PC + 1) + reg_X;
				uint8 &oper = memory[adr];
				reg_S[B_C] = (oper & B_7) == B_7;
				oper <<= 1;
				oper ^= (-reg_S[B_C] ^ oper) & B_0;
				SetFlags_NZ(oper);
				reg_PC += 3;
				return true;
			}
			case 0x6A: { // ROR Accumulator (rotate right)
				reg_S[B_C] = (reg_A & B_0) == B_0; // Sets carry
								   // flag to
								   // whatever
								   // bit 0 is
				reg_A >>= 1; // Shifts right one bit
				reg_A ^= (-reg_S[B_C] ^ reg_A) & B_7; // Changes
								      // bit 7
								      // to
								      // whatever
								      // carry
								      // is
				SetFlags_NZ(reg_A);
				reg_PC++;
				return true;
			}
			case 0x66: { // ROR Zeropage
				uint8 offset = memory[reg_PC + 1];
				uint8 &oper = memory[offset];
				reg_S[B_C] = (oper & B_0) == B_0;
				oper >>= 1;
				oper ^= (-reg_S[B_C] ^ oper) & B_7;
				SetFlags_NZ(oper);
				reg_PC += 2;
				return true;
			}
			case 0x76: { // ROR Zeropage, X
				uint8 offset = memory[reg_PC + 1] + reg_X;
				uint8 &oper = memory[offset];
				reg_S[B_C] = (oper & B_0) == B_0;
				oper >>= 1;
				oper ^= (-reg_S[B_C] ^ oper) & B_7;
				SetFlags_NZ(oper);
				reg_PC += 2;
				return true;
			}
			case 0x6E: { // ROR Absolute
				uint16 adr = AsUInt16(reg_PC + 1);
				uint8 &oper = memory[adr];
				reg_S[B_C] = (oper & B_0) == B_0;
				oper >>= 1;
				oper ^= (-reg_S[B_C] ^ oper) & B_7;
				SetFlags_NZ(oper);
				reg_PC += 3;
				return true;
			}
			case 0x7E: { // ROR Absolute, X
				uint16 adr = AsUInt16(reg_PC + 1) + reg_X;
				uint8 &oper = memory[adr];
				reg_S[B_C] = (oper & B_0) == B_0;
				oper >>= 1;
				oper ^= (-reg_S[B_C] ^ oper) & B_7;
				SetFlags_NZ(oper);
				reg_PC += 3;
				return true;
			}
			case 0x29: { // AND Immediate
				uint8 oper = memory[reg_PC + 1];
				reg_A &= oper;
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0x25: { // AND Zeropage
				uint8 offset = memory[reg_PC + 1];
				reg_A &= memory[offset];
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0x35: { // AND Zeropage, X
				uint8 offset = memory[reg_PC + 1] + reg_X;
				reg_A &= memory[offset];
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0x2D: { // AND Absolute
				uint16 adr = AsUInt16(reg_PC + 1);
				reg_A &= memory[adr];
				SetFlags_NZ(reg_A);
				reg_PC += 3;
				return true;
			}
			case 0x3D: { // AND Absolute, X
				uint16 adr = AsUInt16(reg_PC + 1) + reg_X;
				reg_A &= memory[adr];
				SetFlags_NZ(reg_A);
				reg_PC += 3;
				return true;
			}
			case 0x39: { // AND Absolute, Y
				uint16 adr = AsUInt16(reg_PC + 1) + reg_Y;
				reg_A &= memory[adr];
				SetFlags_NZ(reg_A);
				reg_PC += 3;
				return true;
			}
			case 0x21: { // AND Indexed Indirect, X (Add first then
				     // fetch)
				uint8 offset = memory[reg_PC + 1] + reg_X;
				uint16 adr = AsUInt16(offset);
				reg_A &= memory[adr];
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0x31: { // AND Indirect Indexed, Y (Fetch first
				     // then add)
				uint8 offset = memory[reg_PC + 1];
				uint16 adr = AsUInt16(offset) + reg_Y;
				reg_A &= memory[adr];
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0xC9: { // CMP Immediate
				uint8 oper = memory[reg_PC + 1];
				OP_CMP(oper, reg_A);
				reg_PC += 2;
				return true;
			}
			case 0xC5: { // CMP Zeropage
				uint8 offset = memory[reg_PC + 1];
				uint8 oper = memory[offset];
				OP_CMP(oper, reg_A);
				reg_PC += 2;
				return true;
			}
			case 0xD5: { // CMP Zeropage, X
				uint8 offset = memory[reg_PC + 1] + reg_X;
				uint8 oper = memory[offset];
				OP_CMP(oper, reg_A);
				reg_PC += 2;
				return true;
			}
			case 0xCD: { // CMP Absolute
				uint16 adr = AsUInt16(reg_PC + 1);
				uint8 oper = memory[adr];
				OP_CMP(oper, reg_A);
				reg_PC += 3;
				return true;
			}
			case 0xDD: { // CMP Absolute, X
				uint16 adr = AsUInt16(reg_PC + 1) + reg_X;
				uint8 oper = memory[adr];
				OP_CMP(oper, reg_A);
				reg_PC += 3;
				return true;
			}
			case 0xD9: { // CMP Absolute, Y
				uint16 adr = AsUInt16(reg_PC + 1) + reg_Y;
				uint8 oper = memory[adr];
				OP_CMP(oper, reg_A);
				reg_PC += 3;
				return true;
			}
			case 0xC1: { // CMP Indexed Indirect, X (Add first then
				     // fetch)
				uint8 offset = memory[reg_PC + 1] + reg_X;
				uint16 adr = AsUInt16(offset);
				uint8 oper = memory[adr];
				OP_CMP(oper, reg_A);
				reg_PC += 2;
				return true;
			}
			case 0xD1: { // CMP Indirect Indexed, Y (Fetch first
				     // then add)
				uint8 offset = memory[reg_PC + 1];
				uint16 adr = AsUInt16(offset) + reg_Y;
				uint8 oper = memory[adr];
				OP_CMP(oper, reg_A);
				reg_PC += 2;
				return true;
			}
			case 0xE0: { // CPX Immediate
				uint8 oper = memory[reg_PC + 1];
				OP_CMP(oper, reg_X);
				reg_PC += 2;
				return true;
			}
			case 0xE4: { // CPX Zeropage
				uint8 offset = memory[reg_PC + 1];
				uint8 oper = memory[offset];
				OP_CMP(oper, reg_X);
				reg_PC += 2;
				return true;
			}
			case 0xEC: { // CPX Absolute
				uint16 adr = AsUInt16(reg_PC + 1);
				uint8 oper = memory[adr];
				OP_CMP(oper, reg_X);
				reg_PC += 3;
				return true;
			}
			case 0xC0: { // CPY Immediate
				uint8 oper = memory[reg_PC + 1];
				OP_CMP(oper, reg_Y);
				reg_PC += 2;
				return true;
			}
			case 0xC4: { // CPY Zeropage
				uint8 offset = memory[reg_PC + 1];
				uint8 oper = memory[offset];
				OP_CMP(oper, reg_Y);
				reg_PC += 2;
				return true;
			}
			case 0xCC: { // CPY Absolute
				uint16 adr = AsUInt16(reg_PC + 1);
				uint8 oper = memory[adr];
				OP_CMP(oper, reg_Y);
				reg_PC += 3;
				return true;
			}
			case 0x09: { // ORA Immediate
				uint8 oper = memory[reg_PC + 1];
				reg_A |= oper;
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0x05: { // ORA Zeropage
				uint8 offset = memory[reg_PC + 1];
				reg_A |= memory[offset];
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0x15: { // ORA Zeropage, X
				uint8 offset = memory[reg_PC + 1] + reg_X;
				reg_A |= memory[offset];
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0x0D: { // ORA Absolute
				uint16 adr = AsUInt16(reg_PC + 1);
				reg_A |= memory[adr];
				SetFlags_NZ(reg_A);
				reg_PC += 3;
				return true;
			}
			case 0x1D: { // ORA Absolute, X
				uint16 adr = AsUInt16(reg_PC + 1) + reg_X;
				reg_A |= memory[adr];
				SetFlags_NZ(reg_A);
				reg_PC += 3;
				return true;
			}
			case 0x19: { // ORA Absolute, Y
				uint16 adr = AsUInt16(reg_PC + 1) + reg_Y;
				reg_A |= memory[adr];
				SetFlags_NZ(reg_A);
				reg_PC += 3;
				return true;
			}
			case 0x01: { // ORA Indexed Indirect, X (Add first then
				     // fetch)
				uint8 offset = memory[reg_PC + 1] + reg_X;
				uint16 adr = AsUInt16(offset);
				reg_A |= memory[adr];
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0x11: { // ORA Indirect Indexed, Y (Fetch first
				     // then add)
				uint8 offset = memory[reg_PC + 1];
				uint16 adr = AsUInt16(offset) + reg_Y;
				reg_A |= memory[adr];
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0x49: { // EOR Immediate
				uint8 oper = memory[reg_PC + 1];
				reg_A ^= oper;
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0x45: { // EOR Zeropage
				uint8 offset = memory[reg_PC + 1];
				reg_A ^= memory[offset];
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0x55: { // EOR Zeropage, X
				uint8 offset = memory[reg_PC + 1] + reg_X;
				reg_A ^= memory[offset];
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0x4D: { // EOR Absolute
				uint16 adr = AsUInt16(reg_PC + 1);
				reg_A ^= memory[adr];
				SetFlags_NZ(reg_A);
				reg_PC += 3;
				return true;
			}
			case 0x5D: { // EOR Absolute, X
				uint16 adr = AsUInt16(reg_PC + 1) + reg_X;
				reg_A ^= memory[adr];
				SetFlags_NZ(reg_A);
				reg_PC += 3;
				return true;
			}
			case 0x59: { // EOR Absolute, Y
				uint16 adr = AsUInt16(reg_PC + 1) + reg_Y;
				reg_A ^= memory[adr];
				SetFlags_NZ(reg_A);
				reg_PC += 3;
				return true;
			}
			case 0x41: { // EOR Indexed Indirect, X (Add first then
				     // fetch)
				uint8 offset = memory[reg_PC + 1] + reg_X;
				uint16 adr = AsUInt16(offset);
				reg_A ^= memory[adr];
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0x51: { // EOR Indirect Indexed, Y (Fetch first
				     // then add)
				uint8 offset = memory[reg_PC + 1];
				uint16 adr = AsUInt16(offset) + reg_Y;
				reg_A ^= memory[adr];
				SetFlags_NZ(reg_A);
				reg_PC += 2;
				return true;
			}
			case 0xE6: { // INC Zeropage
				uint8 offset = memory[reg_PC + 1];
				memory[offset]++;
				SetFlags_NZ(memory[offset]);
				reg_PC += 2;
				return true;
			}
			case 0xF6: { // INC Zeropage, X
				uint8 offset = memory[reg_PC + 1] + reg_X;
				memory[offset]++;
				SetFlags_NZ(memory[offset]);
				reg_PC += 2;
				return true;
			}
			case 0xEE: { // INC Absolute
				uint16 adr = AsUInt16(reg_PC + 1);
				memory[adr]++;
				SetFlags_NZ(memory[adr]);
				reg_PC += 3;
				return true;
			}
			case 0xFE: { // INC Absolute, X
				uint16 adr = AsUInt16(reg_PC + 1) + reg_X;
				memory[adr]++;
				SetFlags_NZ(memory[adr]);
				reg_PC += 3;
				return true;
			}
			case 0xE8: { // INX
				reg_X++;
				SetFlags_NZ(reg_X);
				reg_PC++;
				return true;
			}
			case 0xC8: { // INY
				reg_Y++;
				SetFlags_NZ(reg_Y);
				reg_PC++;
				return true;
			}

			default:
				return false;
			};
		}

		void Run()
		{
			running = true;
			reg_PC = AsUInt16(0xFFFC); // Load PC with the reset
						   // vector
			while (running) { // Execute while PC is valid
				if (!Decode()) {
					std::cout
						<< "Op-code not supported, exiting..."
						<< std::endl;
					return;
				}
			}
		}

		void SetMemory(Memory& mem)
		{
			memory = mem;
		}

		void PrintFlags()
		{
			std::cout << "[C: " << +reg_S[B_C] << " | "
				  << "Z:  " << +reg_S[B_Z] << " | "
				  << "N:  " << +reg_S[B_N] << " | "
				  << "O:  " << +reg_S[B_O] << "]" << std::endl;
		}
		void PrintRegisters()
		{
			std::cout << "A: " << +reg_A << std::endl;
			std::cout << "X: " << +reg_X << std::endl;
			std::cout << "Y: " << +reg_Y << std::endl;
		}

	    private:
		/// The definition of "Overflow" on the 6502 is that -
		/// the result of a signed addition or subtraction doesn't fit
		/// into a signed byte.
		///
		/// For addition this means that bit 7 is set; the operation
		/// overflowed into the sign bit. For subtraction this means
		/// that bit 7 is not set; a carry from the 6th place shifted
		/// the sign bit out of its place. Note that overflow can't
		/// occur if the operands have different signs, since it will
		/// always be less than the positive one.
		///
		/// The formula for addition is: { A + B + C }.
		/// The formula for subtraction is: { A + ~B + C }. In other
		/// words if C = 1 it simply results in A - B since - ~B + C is
		/// the two's complement of B. If C = 0 is results in A - B - 1
		/// since the extra bit is missing. Therefore the exact same
		/// logic can be applied to subtraction with the difference
		/// being B is swapped for ~B.

		void OP_ADC(uint8 oper)
		{
			uint result = reg_A + oper + reg_S[B_C];
			reg_S[B_C] = (result & B_8) == B_8 ? 1 : 0; // If 8th
								    // bit is 1,
								    // set carry
			reg_S[B_O] = ((reg_A ^ result) & (oper ^ result) &
				      B_7) != 0; // If the signs of both
						 // operands != the sign of the
						 // result, set overflow
			reg_S[B_N] = (result & B_7) == B_7 ? 1 : 0; // If 7th
								    // bit is 1,
								    // set
								    // negative
			reg_A = result & 0xFF; // 8 LSB -> A
			reg_S[B_Z] = reg_A == 0; // Set status zero

			std::cout << "Flags after add:" << std::endl;
			PrintFlags();
		}

		void OP_CMP(uint8 oper, uint8 reg)
		{
			uint16 result = reg - oper;
			reg_S[B_Z] = oper == reg;
			reg_S[B_N] = (result & B_7) == B_7;
			reg_S[B_C] = (result & B_8) == B_8;
		}

		void OP_SBC(uint8 oper)
		{
			uint8 complement = -oper;
			uint result = reg_A + complement + reg_S[B_C];
			reg_S[B_C] = (result & B_8) == B_8 ? 1 : 0; // If 8th
								    // bit is 1,
								    // set carry
			reg_S[B_O] = ((reg_A ^ result) & (complement ^ result) &
				      B_7) != 0; // Same logic as add
			reg_S[B_N] = (result & B_7) == B_7 ? 1 : 0; // If 7th
								    // bit is 1,
								    // set
								    // negative
			reg_A = result & 0xFF; // 8 LSB -> A
			reg_S[B_Z] = reg_A == 0; // Set status zero

			std::cout << "Flags after sub:" << std::endl;
			PrintFlags();
		}

		void SetFlags_NZ(uint8 reg)
		{
			reg_S[B_N] = (reg & B_7) == B_7 ? 1 : 0; // If 7th bit
								 // is 1, set
								 // negative
			reg_S[B_Z] = reg == 0; // If register is 0, set zero
		}
	};

} // namespace nemu