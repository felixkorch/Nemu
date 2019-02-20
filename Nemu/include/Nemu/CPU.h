#pragma once
#include "Nemu/Stack.h"
#include <cstdint>
#include <vector>
#include <array>
#include <iostream>

namespace nemu {

	typedef std::uint8_t uint8;
	typedef std::uint16_t uint16;
	typedef std::int8_t int8;
	typedef std::uint8_t uint;

	class CPU {

	private:

		constexpr static std::size_t RAM_SIZE = 0xFFFF;
		constexpr static std::size_t STACK_SIZE = 256;

		uint8 reg_x;
		uint8 reg_y;
		uint8 reg_a;

		uint16 reg_pc;
		std::array<uint8, 8> reg_status;

		std::vector<uint8> memory; // RAM
		Stack<uint8, STACK_SIZE> stack;

		// Status bits
		constexpr static uint8 S_CARRY       = 0;
		constexpr static uint8 S_ZERO        = 1;
		constexpr static uint8 S_IRQ_DISABLE = 2;
		//constexpr static uint8 S_BCD         = 3; // Disabled (decimal mode)?
		constexpr static uint8 S_BRK         = 4;
		constexpr static uint8 S_OVF         = 6;
		constexpr static uint8 S_N           = 7;

		constexpr static uint8  B_0 =  (1 << 0);
		constexpr static uint8  B_1 =  (1 << 1);
		constexpr static uint8  B_2 =  (1 << 2);
		constexpr static uint8  B_3 =  (1 << 3);
		constexpr static uint8  B_4 =  (1 << 4);
		constexpr static uint8  B_5 =  (1 << 5);
		constexpr static uint8  B_6 =  (1 << 6);
		constexpr static uint8  B_7 =  (1 << 7);
		constexpr static uint16 B_8 =  (1 << 8);

	public:

		CPU() :
			memory(RAM_SIZE),
			reg_x(0),
			reg_y(0),
			reg_a(0),
			reg_pc(0),
			reg_status{},
			stack(memory.data() + 0x1FF){} // Stack range 0x100 -> 0x1FF

		uint8 ToBitMask(std::array<uint8, 8> array)
		{
			uint8 mask = 0;
			for (uint i = 0; i < 8; i++)
				mask |= (array[i] << i);
			return mask;
		}

		void FromBitMask(uint8 mask, std::array<uint8, 8>& to)
		{
			for (uint i = 0; i < 8; i++)
				to[i] = ((1 << i) & mask) == (1 << i);

		}


		bool Decode()
		{
			switch (memory[reg_pc]) {
			case 0x0: { // BRK
				reg_status[S_IRQ_DISABLE] = 1;
				stack.Push(reg_pc + 2);
				stack.Push(ToBitMask(reg_status));
				reg_pc++;
				return true;
			}
			case 0xA0: { // LDY Immediate
				reg_y = memory[reg_pc + 1];
				LD_SetFlags(reg_y);
				reg_pc += 2;
				return true;
			}
			case 0xA4: { // LDY Zeropage
				uint8 offset = memory[reg_pc + 1];
				reg_y = memory[offset];
				LD_SetFlags(reg_y);
				reg_pc += 2;
				return true;
			}
			case 0xB4: { // LDY Zeropage, X
				uint8 offset = memory[reg_pc + 1] + reg_x;
				reg_y = memory[offset];
				LD_SetFlags(reg_y);
				reg_pc += 2;
				return true;
			}
			case 0xAC: { // LDY Absolute
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1);
				reg_y = memory[offset];
				LD_SetFlags(reg_y);
				reg_pc += 3;
				return true;
			}
			case 0xBC: { // LDY Absolute, X
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1) + reg_x;
				reg_y = memory[offset];
				LD_SetFlags(reg_y);
				reg_pc += 3;
				return true;
			}

			case 0xA2: { // LDX Immediate
				reg_x = memory[reg_pc + 1];
				LD_SetFlags(reg_x);
				reg_pc += 2;
				return true;
			}
			case 0xA6: { // LDX Zeropage
				uint8 offset = memory[reg_pc + 1];
				reg_x = memory[offset];
				LD_SetFlags(reg_x);
				reg_pc += 2;
				return true;
			}
			case 0xB6: { // LDX Zeropage, Y
				uint8 offset = memory[reg_pc + 1] + reg_y;
				reg_x = memory[offset];
				LD_SetFlags(reg_x);
				reg_pc += 2;
				return true;
			}
			case 0xAE: { // LDX Absolute
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1);
				reg_x = memory[offset];
				LD_SetFlags(reg_x);
				reg_pc += 3;
				return true;
			}
			case 0xBE: { // LDX Absolute, Y
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1) + reg_y;
				reg_x = memory[offset];
				LD_SetFlags(reg_x);
				reg_pc += 3;
				return true;
			}
			case 0xEA: { // NOP
				reg_pc++;
				return true;
			}

			case 0x18: { // CLC
				reg_status[S_CARRY] = 0;
				reg_pc += 1;
				return true;
			}
			case 0xA9: { // LDA Immediate
				reg_a = memory[reg_pc + 1];
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0xA5: { // LDA Zeropage
				uint8 offset = memory[reg_pc + 1];
				reg_a = memory[offset];
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0xB5: { // LDA Zeropage, X
				uint8 offset = memory[reg_pc + 1] + reg_x;
				reg_a = memory[offset];
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0xAD: { // LDA Absolute
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1);
				reg_a = memory[offset];
				LD_SetFlags(reg_a);
				reg_pc += 3;
				return true;
			}
			case 0xBD: { // LDA Absolute, X
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1) + reg_x;
				reg_a = memory[offset];
				LD_SetFlags(reg_a);
				reg_pc += 3;
				return true;
			}
			case 0xB9: { // LDA Absolute, Y
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1) + reg_y;
				reg_a = memory[offset];
				LD_SetFlags(reg_a);
				reg_pc += 3;
				return true;
			}
			case 0xA1: { // LDA Indexed Indirect, X (Add first then fetch)
				uint8 offset = memory[reg_pc + 1] + reg_x;
				uint16 adr = *(uint16*)(memory.data() + offset);
				reg_a = memory[adr];
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0xB1: { // LDA Indirect Indexed, Y (Fetch first then add)
				uint8 offset = memory[reg_pc + 1];
				uint16 adr = *(uint16*)(memory.data() + offset) + reg_y;
				reg_a = memory[adr];
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0x69: { // ADC Immediate
				uint8 oper = memory[reg_pc + 1];
				ADC_SetFlags(oper);
				reg_pc += 2;
				return true;
			}
			case 0x65: { // ADC Zeropage
				uint8 offset = memory[reg_pc + 1];
				uint8 oper = memory[offset];
				ADC_SetFlags(oper);
				reg_pc += 2;
				return true;
			}
			case 0x75: { // ADC Zeropage, X
				uint8 offset = memory[reg_pc + 1] + reg_x;
				uint8 oper = memory[offset];
				ADC_SetFlags(oper);
				reg_pc += 2;
				return true;
			}
			case 0x6D: { // ADC Absolute
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1);
				uint8 oper = memory[offset];
				ADC_SetFlags(oper);
				reg_pc += 3;
				return true;
			}
			case 0x7D: { // ADC Absolute, X
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1) + reg_x;
				uint8 oper = memory[offset];
				ADC_SetFlags(oper);
				reg_pc += 3;
				return true;
			}
			case 0x79: { // ADC Absolute, Y
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1) + reg_y;
				uint8 oper = memory[offset];
				ADC_SetFlags(oper);
				reg_pc += 3;
				return true;
			}
			case 0x61: { // ADC Indexed Indirect, X (Add first then fetch)
				uint8 offset = memory[reg_pc + 1] + reg_x;
				uint16 adr = *(uint16*)(memory.data() + offset);
				uint8 oper = memory[adr];
				ADC_SetFlags(oper);
				reg_pc += 2;
				return true;
			}
			case 0x71: { // ADC Indirect Indexed, Y (Fetch first then add)
				uint8 offset = memory[reg_pc + 1];
				uint16 adr = *(uint16*)(memory.data() + offset) + reg_y;
				uint8 oper = memory[adr];
				ADC_SetFlags(oper);
				reg_pc += 2;
				return true;
			}
			case 0xE9: { // SBC Immediate
				uint8 oper = memory[reg_pc + 1];
				SBC_SetFlags(oper);
				reg_pc += 2;
				return true;
			}
			case 0xE5: { // SBC Zeropage
				uint8 offset = memory[reg_pc + 1];
				uint8 oper = memory[offset];
				SBC_SetFlags(oper);
				reg_pc += 2;
				return true;
			}
			case 0xF5: { // SBC Zeropage, X
				uint8 offset = memory[reg_pc + 1] + reg_x;
				uint8 oper = memory[offset];
				SBC_SetFlags(oper);
				reg_pc += 2;
				return true;
			}
			case 0xED: { // SBC Absolute
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1);
				uint8 oper = memory[offset];
				SBC_SetFlags(oper);
				reg_pc += 3;
				return true;
			}
			case 0xFD: { // SBC Absolute, X
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1) + reg_x;
				uint8 oper = memory[offset];
				SBC_SetFlags(oper);
				reg_pc += 3;
				return true;
			}
			case 0xF9: { // SBC Absolute, Y
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1) + reg_y;
				uint8 oper = memory[offset];
				SBC_SetFlags(oper);
				reg_pc += 3;
				return true;
			}
			case 0xE1: { // SBC Indexed Indirect, X (Add first then fetch)
				uint8 offset = memory[reg_pc + 1] + reg_x;
				uint16 adr = *(uint16*)(memory.data() + offset);
				uint8 oper = memory[adr];
				SBC_SetFlags(oper);
				reg_pc += 2;
				return true;
			}
			case 0xF1: { // SBC Indirect Indexed, Y (Fetch first then add)
				uint8 offset = memory[reg_pc + 1];
				uint16 adr = *(uint16*)(memory.data() + offset) + reg_y;
				uint8 oper = memory[adr];
				SBC_SetFlags(oper);
				reg_pc += 2;
				return true;
			}
			case 0x85: { // STA Zeropage
				uint8 offset = memory[reg_pc + 1];
				memory[offset] = reg_a;
				reg_pc += 2;
				return true;
			}
			case 0x95: { // STA Zeropage, X
				uint8 offset = memory[reg_pc + 1] + reg_x;
				memory[offset] = reg_a;
				reg_pc += 2;
				return true;
			}
			case 0x8D: { // STA Absolute
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1);
				memory[adr] = reg_a;
				reg_pc += 3;
				return true;
			}
			case 0x9D: { // STA Absolute, X
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1) + reg_x;
				memory[adr] = reg_a;
				reg_pc += 3;
				return true;
			}
			case 0x99: { // STA Absolute, Y
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1) + reg_y;
				memory[adr] = reg_a;
				reg_pc += 3;
				return true;
			}
			case 0x81: { // STA Indexed Indirect, X (Add first then fetch)
				uint8 offset = memory[reg_pc + 1] + reg_x;
				uint16 adr = *(uint16*)(memory.data() + offset);
				memory[adr] = reg_a;
				reg_pc += 2;
				return true;
			}
			case 0x91: { // STA Indirect Indexed, Y (Fetch first then add)
				uint8 offset = memory[reg_pc + 1];
				uint16 adr = *(uint16*)(memory.data() + offset) + reg_y;
				memory[adr] = reg_a;
				reg_pc += 2;
				return true;
			}
			case 0x86: { // STX Zeropage
				uint8 offset = memory[reg_pc + 1];
				memory[offset] = reg_x;
				reg_pc += 2;
				return true;
			}
			case 0x96: { // STX Zeropage, Y
				uint8 offset = memory[reg_pc + 1] + reg_y;
				memory[offset] = reg_x;
				reg_pc += 2;
				return true;
			}
			case 0x8E: { // STX, Absolute
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1);
				memory[adr] = reg_x;
				reg_pc += 3;
				return true;
			}
			case 0x84: { // STY Zeropage
				uint8 offset = memory[reg_pc + 1];
				memory[offset] = reg_y;
				reg_pc += 2;
				return true;
			}
			case 0x94: { // STY Zeropage, X
				uint8 offset = memory[reg_pc + 1] + reg_x;
				memory[offset] = reg_y;
				reg_pc += 2;
				return true;
			}
			case 0x8C: { // STY Absolute
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1);
				memory[adr] = reg_y;
				reg_pc += 3;
				return true;
			}
			case 0x4C: { // JMP Absolute
				reg_pc = *(uint16*)(memory.data() + reg_pc + 1);
				return true;
			}
			case 0x6C: { // JMP Indirect
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1);
				reg_pc = memory[offset];
				return true;
			}
			case 0x20: { // JSR
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1);
				stack.Push(reg_pc);
				reg_pc = offset;
				return true;
			}
			case 0x48: { // PHA
				stack.Push(reg_a);
				reg_pc++;
				return true;
			}
			case 0x08: { // PHP
				stack.Push(ToBitMask(reg_status));
				reg_pc++;
				return true;
			}
			case 0x68: { // PLA
				reg_a = stack.Pop();
				LD_SetFlags(reg_a);
				reg_pc++;
				return true;
			}
			case 0x28: { // PLP
				FromBitMask(stack.Pop(), reg_status);
				reg_pc++;
				return true;
			}
			case 0x40: { // RTI
				FromBitMask(stack.Pop(), reg_status);
				reg_pc = stack.Pop();
				return true;
			}
			case 0x60: { // RTS
				reg_pc = stack.Pop();
				reg_pc++;
				return true;
			}
			case 0xF0: { // BEQ
				int8 oper = memory[reg_pc + 1];
				reg_pc += reg_status[S_ZERO] ? oper : 2;
				return true;
			}
			case 0xCA: { // DEX, Decrement index X by 1
				reg_x--;
				reg_status[S_ZERO] = reg_x == 0 ? 1 : 0;
				reg_pc++;
				return true;
			}
			case 0x8A: { // TXA (Transfer X -> A)
				reg_a = reg_x;
				reg_pc++;
				return true;
			}
			case 0xAA: { // TAX (Transfer A -> X)
				reg_x = reg_a;
				reg_pc++;
				return true;
			}
			case 0xA8: { // TAY (Transfer A -> Y)
				reg_y = reg_a;
				reg_pc++;
				return true;
			}
			case 0xBA: { // TSX (Transfer SP -> X)
				reg_x = stack.GetSP();
				reg_pc++;
				return true;
			}
			case 0x9A: { // TXS (Transfer X -> SP)
				stack.SetSP(reg_x);
				reg_pc++;
				return true;
			}
			case 0x98: { // TYA (Transfer Y -> A)
				reg_a = reg_y;
				reg_pc++;
				return true;
			}

			case 0x10: { // BPL, Branch on result plus
				int8 oper = memory[reg_pc + 1];
				reg_pc += (reg_status[S_N] == 0) ? oper : 1;
				return true;
			}

			case 0x29: { // AND Immediate
				uint8 oper = memory[reg_pc + 1];
				reg_a &= oper;
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0x25: { // AND Zeropage
				uint8 offset = memory[reg_pc + 1];
				reg_a &= memory[offset];
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0x35: { // AND Zeropage, X
				uint8 offset = memory[reg_pc + 1] + reg_x;
				reg_a &= memory[offset];
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0x2D: { // AND Absolute
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1);
				reg_a &= memory[adr];
				LD_SetFlags(reg_a);
				reg_pc += 3;
				return true;
			}
			case 0x3D: { // AND Absolute, X
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1) + reg_x;
				reg_a &= memory[adr];
				LD_SetFlags(reg_a);
				reg_pc += 3;
				return true;
			}
			case 0x39: { // AND Absolute, Y
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1) + reg_y;
				reg_a &= memory[adr];
				LD_SetFlags(reg_a);
				reg_pc += 3;
				return true;
			}
			case 0x21: { // AND Indexed Indirect, X (Add first then fetch)
				uint8 offset = memory[reg_pc + 1] + reg_x;
				uint16 adr = *(uint16*)(memory.data() + offset);
				reg_a &= memory[adr];
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0x31: { // AND Indirect Indexed, Y (Fetch first then add)
				uint8 offset = memory[reg_pc + 1];
				uint16 adr = *(uint16*)(memory.data() + offset) + reg_y;
				reg_a &= memory[adr];
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0x09: { // ORA Immediate
				uint8 oper = memory[reg_pc + 1];
				reg_a |= oper;
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0x05: { // ORA Zeropage
				uint8 offset = memory[reg_pc + 1];
				reg_a |= memory[offset];
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0x15: { // ORA Zeropage, X
				uint8 offset = memory[reg_pc + 1] + reg_x;
				reg_a |= memory[offset];
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0x0D: { // ORA Absolute
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1);
				reg_a |= memory[adr];
				LD_SetFlags(reg_a);
				reg_pc += 3;
				return true;
			}
			case 0x1D: { // ORA Absolute, X
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1) + reg_x;
				reg_a |= memory[adr];
				LD_SetFlags(reg_a);
				reg_pc += 3;
				return true;
			}
			case 0x19: { // ORA Absolute, Y
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1) + reg_y;
				reg_a |= memory[adr];
				LD_SetFlags(reg_a);
				reg_pc += 3;
				return true;
			}
			case 0x01: { // ORA Indexed Indirect, X (Add first then fetch)
				uint8 offset = memory[reg_pc + 1] + reg_x;
				uint16 adr = *(uint16*)(memory.data() + offset);
				reg_a |= memory[adr];
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0x11: { // ORA Indirect Indexed, Y (Fetch first then add)
				uint8 offset = memory[reg_pc + 1];
				uint16 adr = *(uint16*)(memory.data() + offset) + reg_y;
				reg_a |= memory[adr];
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0x49: { // EOR Immediate
				uint8 oper = memory[reg_pc + 1];
				reg_a ^= oper;
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0x45: { // EOR Zeropage
				uint8 offset = memory[reg_pc + 1];
				reg_a ^= memory[offset];
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0x55: { // EOR Zeropage, X
				uint8 offset = memory[reg_pc + 1] + reg_x;
				reg_a ^= memory[offset];
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0x4D: { // EOR Absolute
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1);
				reg_a ^= memory[adr];
				LD_SetFlags(reg_a);
				reg_pc += 3;
				return true;
			}
			case 0x5D: { // EOR Absolute, X
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1) + reg_x;
				reg_a ^= memory[adr];
				LD_SetFlags(reg_a);
				reg_pc += 3;
				return true;
			}
			case 0x59: { // EOR Absolute, Y
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1) + reg_y;
				reg_a ^= memory[adr];
				LD_SetFlags(reg_a);
				reg_pc += 3;
				return true;
			}
			case 0x41: { // EOR Indexed Indirect, X (Add first then fetch)
				uint8 offset = memory[reg_pc + 1] + reg_x;
				uint16 adr = *(uint16*)(memory.data() + offset);
				reg_a ^= memory[adr];
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0x51: { // EOR Indirect Indexed, Y (Fetch first then add)
				uint8 offset = memory[reg_pc + 1];
				uint16 adr = *(uint16*)(memory.data() + offset) + reg_y;
				reg_a ^= memory[adr];
				LD_SetFlags(reg_a);
				reg_pc += 2;
				return true;
			}
			case 0xE6: { // INC Zeropage
				uint8 offset = memory[reg_pc + 1];
				memory[offset]++;
				LD_SetFlags(memory[offset]);
				reg_pc += 2;
				return true;
			}
			case 0xF6: { // INC Zeropage, X
				uint8 offset = memory[reg_pc + 1] + reg_x;
				memory[offset]++;
				LD_SetFlags(memory[offset]);
				reg_pc += 2;
				return true;
			}
			case 0xEE: { // INC Absolute
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1);
				memory[adr]++;
				LD_SetFlags(memory[adr]);
				reg_pc += 3;
				return true;
			}
			case 0xFE: { // INC Absolute, X
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1) + reg_x;
				memory[adr]++;
				LD_SetFlags(memory[adr]);
				reg_pc += 3;
				return true;
			}
			case 0xE8: { // INX
				reg_x++;
				LD_SetFlags(reg_x);
				reg_pc++;
				return true;
			}
			case 0xC8: { // INY
				reg_y++;
				LD_SetFlags(reg_y);
				reg_pc ++;
				return true;
			}

			default:
				return false;
			};
		}

		void LoadProgram(std::vector<uint8> program, uint16 offset)
		{
			std::copy(program.begin(), program.end(), memory.begin() + offset); // Copy the program into RAM
			memory[0xFFFC] = 0x00; // temp
			memory[0xFFFD] = 0x80; // temp
			reg_pc = *(uint16*)(memory.data() + 0xFFFC); // Load PC with the reset vector

			while (reg_pc >= 0 && reg_pc < offset + program.size()) { // Execute while PC is valid
				if (!Decode()) {
					std::cout << "Op-code not supported, exiting..." << std::endl;
					break;
				}
			}

			std::cout << "Program done executing" << std::endl;
		}

		std::vector<uint8>& GetRAM()
		{
			return memory;
		}

		void PrintRegisters()
		{
			std::cout <<
				"[C: "  <<  +reg_status[S_CARRY] << " | " <<
				"Z:  "  <<  +reg_status[S_ZERO]  << " | " <<
				"N:  "  <<  +reg_status[S_N]     << " | " <<
				"O:  "  <<  +reg_status[S_OVF]   << "]"   <<
			std::endl;
		}

private:

	/// The definition of "Overflow" on the 6502 is that -
	/// the result of a signed addition or subtraction doesn't fit into a signed byte.
	///
	/// For addition this means that bit 7 is set; the operation overflowed into the sign bit.
	/// For subtraction this means that bit 7 is not set; a carry from the 6th place shifted the sign bit out of its place.
	/// Note that overflow can't occur if the operands have different signs, since it will always be less than the positive one.
	///
	/// The formula for addition is: { A + B + C }.
	/// The formula for subtraction is: { A + ~B + C }. In other words if C = 1 it simply results in A - B since -
	/// ~B + C is the two's complement of B. If C = 0 is results in A - B - 1 since the extra bit is missing.
	/// Therefore the exact same logic can be applied to subtraction with the difference being B is swapped for ~B.

	void ADC_SetFlags(uint8 oper)
	{
		uint result = reg_a + oper + reg_status[S_CARRY];
		reg_status[S_CARRY] = (result & B_8) == B_8 ? 1 : 0;                 // If 8th bit is 1, set carry
		reg_status[S_OVF] = ((reg_a ^ result) & (oper ^ result) & B_7) != 0; // If the signs of both operands != the sign of the result, set overflow
		reg_status[S_N] = (result & B_7) == B_7 ? 1 : 0;                     // If 7th bit is 1, set negative
		reg_a = result & 0xFF;                                               // 8 LSB -> A
		reg_status[S_ZERO] = reg_a == 0;                                     // Set status zero
		PrintRegisters();
	}

	void SBC_SetFlags(uint8 oper)
	{
		uint8 complement = -oper;
		uint result = reg_a + complement + reg_status[S_CARRY];
		reg_status[S_CARRY] = (result & B_8) == B_8 ? 1 : 0;                       // If 8th bit is 1, set carry
		reg_status[S_OVF] = ((reg_a ^ result) & (complement ^ result) & B_7) != 0; // Same logic as add
		reg_status[S_N] = (result & B_7) == B_7 ? 1 : 0;                           // If 7th bit is 1, set negative
		reg_a = result & 0xFF;                                                     // 8 LSB -> A
		reg_status[S_ZERO] = reg_a == 0;                                           // Set status zero
		PrintRegisters();
	}

	void LD_SetFlags(uint8 reg)
	{
		reg_status[S_N] = (reg & B_7) == B_7 ? 1 : 0; // If 7th bit is 1, set negative
		reg_status[S_ZERO] = reg == 0;                // If register is 0, set zero
	}
};

} // namespace