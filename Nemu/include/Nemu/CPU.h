#pragma once
#include "Nemu/Stack.h"
#include <cstdint>
#include <vector>
#include <array>
#include <iostream>

namespace nemu {

	class CPU {

	private:

		constexpr static std::size_t RAM_SIZE = 0xffff;
		constexpr static std::size_t STACK_SIZE = 256;

		std::uint8_t reg_x;
		std::uint8_t reg_y;
		std::uint8_t reg_a;

		std::uint16_t reg_pc;
		std::array<std::uint8_t, 8> reg_status;

		std::vector<std::uint8_t> random_access_memory;
		Stack<STACK_SIZE> stack;

		// Status bits
		constexpr static std::uint8_t S_CARRY = 0;
		constexpr static std::uint8_t S_ZERO = 1;
		constexpr static std::uint8_t S_IRQ_DISABLE = 2;
		//constexpr static std::uint8_t S_BCD       = 3; // Disabled (decimal mode)?
		constexpr static std::uint8_t S_BRK = 4;
		constexpr static std::uint8_t S_OVF = 6;
		constexpr static std::uint8_t S_NEG = 7;

		constexpr static std::uint8_t B_1 = (1 << 0);
		constexpr static std::uint8_t B_2 = (1 << 1);
		constexpr static std::uint8_t B_3 = (1 << 2);
		constexpr static std::uint8_t B_4 = (1 << 3);
		constexpr static std::uint8_t B_5 = (1 << 4);
		constexpr static std::uint8_t B_6 = (1 << 5);
		constexpr static std::uint8_t B_7 = (1 << 6);
		constexpr static std::uint8_t B_8 = (1 << 7);
		constexpr static std::uint16_t B_9 = (1 << 8);

	public:

		CPU() :
			random_access_memory(RAM_SIZE),
			reg_x(0),
			reg_y(0),
			reg_a(0),
			reg_pc(0),
			reg_status{} {}

		std::uint8_t ToBitMask(std::array<std::uint8_t, 8> array)
		{
			std::uint8_t mask = 0;
			for (unsigned int i = 0; i < 8; i++)
				mask |= (array[i] << i);
			return mask;
		}

		std::vector<std::uint8_t> FromBitMask(std::uint8_t mask, std::array<std::uint8_t, 8> to)
		{
			for (unsigned int i = 0; i < 8; i++)
				to[i] = ((1 << i) & mask) == (1 << i);
		}


		bool Decode()
		{
			switch (random_access_memory[reg_pc]) {
			case 0x0: { // BRK
				reg_status[S_IRQ_DISABLE] = 1;
				stack.Push(random_access_memory[reg_pc + 2]);
				stack.Push(ToBitMask(reg_status));
				return true;
			}
			case 0xA0: { // LDY Immediate
				reg_y = random_access_memory[reg_pc + 1];
				reg_status[S_NEG] = (reg_y & B_8 == B_8) ? 1 : 0; // Negative if 8th bit is set
				reg_status[S_ZERO] = reg_y == 0 ? 1 : 0;
				reg_pc += 2;
				return true;
			}
			case 0x18: { // CLC
				reg_status[S_CARRY] = 0;
				reg_pc += 1;
				return true;
			}
			case 0xA9: { // LDA Immediate
				reg_a = random_access_memory[reg_pc + 1];
				reg_status[S_NEG] = (reg_a & B_8 == B_8) ? 1 : 0; // Negative if 8th bit is set
				reg_status[S_ZERO] = reg_a == 0 ? 1 : 0;
				reg_pc += 2;
				return true;
			}
			case 0x69: { // ADC Immediate
				std::uint8_t oper = random_access_memory[reg_pc + 1];
				unsigned int result = reg_a + oper + reg_status[S_CARRY];
				reg_status[S_CARRY] = (result & B_9) == B_9 ? 1 : 0; // If 9th bit is 1, set carry
				reg_status[S_NEG] = (result & B_8) == B_8 ? 1 : 0; // Negative if 8th bit is set
				reg_a = result & 0xff;
				reg_pc += 2;
				return true;
			}
			case 0x8D: { // STA Absolute
				std::uint16_t adr = *(std::uint16_t*)(random_access_memory.data() + reg_pc + 1);
				random_access_memory[adr] = reg_a;
				reg_pc += 3;
				return true;
			}
			case 0x4C: { // JMP Absolute
				reg_pc = *(std::uint16_t*)(random_access_memory.data() + reg_pc + 1);
				return true;
			}
			case 0xF0: { // BEQ
				std::uint8_t oper = random_access_memory[reg_pc + 1];
				reg_pc += reg_status[S_ZERO] ? oper : 2;
				return true;
			}
			case 0xCA: { // DEX, Decrement index by 1
				reg_x--;
				reg_status[S_ZERO] = reg_x == 0 ? 1 : 0;
				reg_pc++;
				return true;
			}
			case 0xA2: { // LDX Immediate
				reg_x = random_access_memory[reg_pc + 1];
				reg_status[S_NEG] = (reg_x & B_8 == B_8) ? 1 : 0; // Negative if 8th bit is set
				reg_status[S_ZERO] = reg_x == 0 ? 1 : 0;
				reg_pc += 2;
				return true;
			}
			case 0x8E: { // STX Absolute
				std::uint16_t adr = *(std::uint16_t*)(random_access_memory.data() + reg_pc + 1);
				random_access_memory[adr] = reg_x;
				reg_pc += 3;
				return true;
			}
			case 0x9D: { // STA Absolute, X
				std::uint16_t adr = *(std::uint16_t*)(random_access_memory.data() + reg_pc + 1);
				std::uint16_t address = adr + reg_x;
				random_access_memory[address] = reg_a;
				reg_pc += 3;
				return true;
			}
			case 0x8A: { // TXA (Transfer X -> A)
				reg_a = reg_x;
				reg_pc++;
				return true;
			}
			default:
				return false;
			};
		}

		void LoadProgram(std::vector<std::uint8_t> program, std::uint16_t offset)
		{
			std::copy(program.begin(), program.end(), random_access_memory.begin() + offset); // Copy the program into RAM
			random_access_memory[0xfffc] = 0; // temp
			random_access_memory[0xfffd] = 0x80; // temp
			reg_pc = *(std::uint16_t*)(random_access_memory.data() + 0xfffc); // Load PC with the reset vector

			while (reg_pc >= 0 && reg_pc < offset + program.size()) { // Execute while PC is valid
				if (!Decode()) {
					std::cout << "Op-code not supported, exiting..." << std::endl;
					break;
				}
			}

			std::cout << "Program done executing" << std::endl;
		}

		const std::vector<std::uint8_t>& GetRAM()
		{
			return random_access_memory;
		}
	};

}