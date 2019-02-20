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

		constexpr static uint8 B_1 =  (1 << 0);
		constexpr static uint8 B_2 =  (1 << 1);
		constexpr static uint8 B_3 =  (1 << 2);
		constexpr static uint8 B_4 =  (1 << 3);
		constexpr static uint8 B_5 =  (1 << 4);
		constexpr static uint8 B_6 =  (1 << 5);
		constexpr static uint8 B_7 =  (1 << 6);
		constexpr static uint8 B_8 =  (1 << 7);
		constexpr static uint16 B_9 = (1 << 8);

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

		std::vector<uint8> FromBitMask(uint8 mask, std::array<uint8, 8> to)
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
				reg_status[S_N] = (reg_y & B_8) == B_8 ? 1 : 0; // If 8th bit is 1, set negative
				reg_status[S_ZERO] = reg_y == 0;                // If register is 0, set zero
				reg_pc += 2;
				return true;
			}
			case 0xA4: { // LDY Zeropage
				uint8 offset = memory[reg_pc + 1];
				reg_y = memory[offset];
				reg_status[S_N] = (reg_y & B_8) == B_8 ? 1 : 0; // If 8th bit is 1, set negative
				reg_status[S_ZERO] = reg_y == 0;                // If register is 0, set zero
				reg_pc += 2;
				return true;
			}
			case 0xB4: { // LDY Zeropage, X
				uint8 offset = memory[reg_pc + 1] + reg_x;
				reg_y = memory[offset];
				reg_status[S_N] = (reg_y & B_8) == B_8 ? 1 : 0; // If 8th bit is 1, set negative
				reg_status[S_ZERO] = reg_y == 0;                // If register is 0, set zero
				reg_pc += 2;
				return true;
			}
			case 0xAC: { // LDY Absolute
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1);
				reg_y = memory[offset];
				reg_status[S_N] = (reg_y & B_8) == B_8 ? 1 : 0; // If 8th bit is 1, set negative
				reg_status[S_ZERO] = reg_y == 0;                // If register is 0, set zero
				reg_pc += 3;
				return true;
			}
			case 0xBC: { // LDY Absolute, X
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1) + reg_x;
				reg_y = memory[offset];
				reg_status[S_N] = (reg_y & B_8) == B_8 ? 1 : 0; // If 8th bit is 1, set negative
				reg_status[S_ZERO] = reg_y == 0;                // If register is 0, set zero
				reg_pc += 3;
				return true;
			}

			case 0xA2: { // LDX Immediate
				reg_x = memory[reg_pc + 1];
				reg_status[S_N] = (reg_x & B_8) == B_8 ? 1 : 0; // If 8th bit is 1, set negative
				reg_status[S_ZERO] = reg_x == 0;                // If register is 0, set zero
				reg_pc += 2;
				return true;
			}
			case 0xA6: { // LDX Zeropage
				uint8 offset = memory[reg_pc + 1];
				reg_x = memory[offset];
				reg_status[S_N] = (reg_x & B_8) == B_8 ? 1 : 0; // If 8th bit is 1, set negative
				reg_status[S_ZERO] = reg_x == 0;                // If register is 0, set zero
				reg_pc += 2;
				return true;
			}
			case 0xB6: { // LDX Zeropage, Y
				uint8 offset = memory[reg_pc + 1] + reg_y;
				reg_x = memory[offset];
				reg_status[S_N] = (reg_x & B_8) == B_8 ? 1 : 0; // If 8th bit is 1, set negative
				reg_status[S_ZERO] = reg_x == 0;                // If register is 0, set zero
				reg_pc += 2;
				return true;
			}
			case 0xAE: { // LDX Absolute
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1);
				reg_x = memory[offset];
				reg_status[S_N] = (reg_x & B_8) == B_8 ? 1 : 0; // If 8th bit is 1, set negative
				reg_status[S_ZERO] = reg_x == 0;                // If register is 0, set zero
				reg_pc += 3;
				return true;
			}
			case 0xBE: { // LDX Absolute, Y
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1) + reg_y;
				reg_x = memory[offset];
				reg_status[S_N] = (reg_x & B_8) == B_8 ? 1 : 0; // If 8th bit is 1, set negative
				reg_status[S_ZERO] = reg_x == 0;                // If register is 0, set zero
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
				reg_status[S_N] = (reg_y & B_8) == B_8 ? 1 : 0;
				reg_status[S_ZERO] = reg_a == 0 ? 1 : 0;
				reg_pc += 2;
				return true;
			}
			case 0xA5: { // LDA Zeropage
				uint8 offset = memory[reg_pc + 1];
				reg_a = memory[offset];
				reg_status[S_N] = (reg_a & B_8) == B_8 ? 1 : 0; // If 8th bit is 1, set negative
				reg_status[S_ZERO] = reg_a == 0;                // If register is 0, set zero
				reg_pc += 2;
				return true;
			}
			case 0xB5: { // LDA Zeropage, X
				uint8 offset = memory[reg_pc + 1] + reg_x;
				reg_a = memory[offset];
				reg_status[S_N] = (reg_a & B_8) == B_8 ? 1 : 0; // If 8th bit is 1, set negative
				reg_status[S_ZERO] = reg_a == 0;                // If register is 0, set zero
				reg_pc += 2;
				return true;
			}
			case 0xAD: { // LDA Absolute
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1);
				reg_a = memory[offset];
				reg_status[S_N] = (reg_a & B_8) == B_8 ? 1 : 0; // If 8th bit is 1, set negative
				reg_status[S_ZERO] = reg_a == 0;                // If register is 0, set zero
				reg_pc += 3;
				return true;
			}
			case 0xBD: { // LDA Absolute, X
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1) + reg_x;
				reg_a = memory[offset];
				reg_status[S_N] = (reg_a & B_8) == B_8 ? 1 : 0; // If 8th bit is 1, set negative
				reg_status[S_ZERO] = reg_a == 0;                // If register is 0, set zero
				reg_pc += 3;
				return true;
			}
			case 0xB9: { // LDA Absolute, Y
				uint16 offset = *(uint16*)(memory.data() + reg_pc + 1) + reg_y;
				reg_a = memory[offset];
				reg_status[S_N] = (reg_a & B_8) == B_8 ? 1 : 0; // If 8th bit is 1, set negative
				reg_status[S_ZERO] = reg_a == 0;                // If register is 0, set zero
				reg_pc += 3;
				return true;
			}
			case 0xA1: { // LDA Indexed Indirect, X (Add first then fetch)
				uint8 offset = memory[reg_pc + 1] + reg_x;
				uint16 adr = *(uint16*)(memory.data() + offset);
				reg_a = memory[adr];
				reg_status[S_N] = (reg_a & B_8) == B_8 ? 1 : 0; // If 8th bit is 1, set negative
				reg_status[S_ZERO] = reg_a == 0;                // If register is 0, set zero
				reg_pc += 2;
				return true;
			}
			case 0xB1: { // LDA Indirect Indexed, Y(Fetch first then add)
				uint8 offset = memory[reg_pc + 1];
				uint16 adr = *(uint16*)(memory.data() + offset) + reg_y;
				reg_a = memory[adr];
				reg_status[S_N] = (reg_a & B_8) == B_8 ? 1 : 0; // If 8th bit is 1, set negative
				reg_status[S_ZERO] = reg_a == 0;                // If register is 0, set zero
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
			case 0x8D: { // STA Absolute
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1);
				memory[adr] = reg_a;
				reg_pc += 3;
				return true;
			}
			case 0x4C: { // JMP Absolute
				reg_pc = *(uint16*)(memory.data() + reg_pc + 1);
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
			case 0x8E: { // STX Absolute
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1);
				memory[adr] = reg_x;
				reg_pc += 3;
				return true;
			}
			case 0x9D: { // STA Absolute, X
				uint16 adr = *(uint16*)(memory.data() + reg_pc + 1);
				uint16 address = adr + reg_x;
				memory[address] = reg_a;
				reg_pc += 3;
				return true;
			}
			case 0x8A: { // TXA (Transfer X -> A)
				reg_a = reg_x;
				reg_pc++;
				return true;
			}

			case 0x10: { // BPL, Branch on result plus
				int8 oper = memory[reg_pc + 1];
				reg_pc += (reg_status[S_N] == 0) ? oper : 1;
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

	void ADC_SetFlags(uint8 oper)
	{
		uint result = reg_a + oper + reg_status[S_CARRY];
		reg_status[S_CARRY] = (result & B_9) == B_9 ? 1 : 0;                 // If 9th bit is 1, set carry
		reg_status[S_OVF] = ((reg_a ^ result) & (oper ^ result) & B_8) != 0; // If the signs of both operands != the sign of the result, set overflow
		reg_status[S_N] = (result & B_8) == B_8 ? 1 : 0;                     // If 8th bit is 1, set negative
		reg_a = result & 0xff;                                               // 8 LSB -> A
		reg_status[S_ZERO] = reg_a == 0;                                     // Set status zero
		PrintRegisters();
	}

	void SBC_SetFlags(uint8 oper)
	{
		uint8 complement = -oper;                                                  // Difference = A + (one's complement of B) + C
		uint result = reg_a + complement + reg_status[S_CARRY];
		reg_status[S_CARRY] = (result & B_9) == B_9 ? 1 : 0;                       // If 9th bit is 1, set carry
		reg_status[S_OVF] = ((reg_a ^ result) & (complement ^ result) & B_8) != 0; // Same logic as add
		reg_status[S_N] = (result & B_8) == B_8 ? 1 : 0;                           // If 8th bit is 1, set negative
		reg_a = result & 0xff;                                                     // 8 LSB -> A
		reg_status[S_ZERO] = reg_a == 0;                                           // Set status zero
		PrintRegisters();
	}
};

} // namespace