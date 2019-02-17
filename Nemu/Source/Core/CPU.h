#pragma once
#include "Stack.h"
#include <cstdint>
#include <vector>

class CPU {

private:

	constexpr static std::size_t RAM_SIZE = 2000;
	constexpr static std::size_t STACK_SIZE = 256;

	std::uint8_t reg_x;
	std::uint8_t reg_y;
	std::uint8_t reg_a;

	std::uint8_t reg_pc;
	std::uint8_t reg_status;

	std::vector<std::uint8_t> instruction_memory;
	std::vector<std::uint8_t> random_access_memory;
	
	Stack<STACK_SIZE> stack;

	// Status bits
	constexpr static std::uint8_t ST_CARRY       = (1 << 0);
	constexpr static std::uint8_t ST_ZERO        = (1 << 1);
	constexpr static std::uint8_t ST_IRQ_DISABLE = (1 << 2);
	//constexpr static std::uint8_t ST_BCD       = (1 << 3); // Disabled (decimal mode)?
	constexpr static std::uint8_t ST_BRK         = (1 << 4);
	constexpr static std::uint8_t ST_OVF         = (1 << 6);
	constexpr static std::uint8_t ST_NEG         = (1 << 7);

public:

	CPU() :
		random_access_memory(RAM_SIZE),
		reg_x(0),
		reg_y(0),
		reg_a(0),
		reg_pc(0),
		reg_status(0) {}

	bool Decode()
	{
		switch (instruction_memory[reg_pc]) {
		case 0x0: { // BRK
			reg_status |= ST_IRQ_DISABLE;
			stack.Push(reg_pc + 2);
			stack.Push(reg_status);
			return true;
		}
		case 0xA0: { // LDY
			reg_y = instruction_memory[reg_pc + 1];
			reg_pc += 2;
			return true;
		}
		case 0x18: { // CLC
			reg_status &= ~ST_CARRY;
			reg_pc += 1;
			return true;
		}
		case 0xA9: { // LDA IMMEDIATE
			reg_a = instruction_memory[reg_pc + 1];
			if (reg_a & (1 << 7))
				reg_status |= ST_NEG;
			else
				reg_status &= ~ST_NEG;

			if (reg_a == 0)
				reg_status |= ST_ZERO;
			else
				reg_status &= ~ST_ZERO;
			reg_pc += 2;
			return true;
		}

		case 0x69: { // ADC IMMEDIATE
			std::uint8_t oper = instruction_memory[reg_pc + 1];

			std::uint8_t carry = ((reg_status & ST_CARRY) == ST_CARRY) ? 1 : 0;
			unsigned int result = reg_a + oper + carry;

			if ((result & 0x100) == 0x100) // If 9th bit is 1, set carry
				reg_status |= ST_CARRY;
			else
				reg_status &= ~ST_CARRY;
			
			if ((result & 0x80) == 0x80) // If 8th bit is 1, set negative
				reg_status |= ST_NEG;
			else
				reg_status &= ~ST_NEG;

			reg_a = result & 0xff;

			reg_pc += 2;
			return true;
		}

		case 0x8D: { // STA Absolute
			std::uint16_t adr = *(std::uint16_t*)(instruction_memory.data() + reg_pc + 1);
			random_access_memory[adr] = reg_a;
			reg_pc += 3;
			return true;
		}

		default:
			return false;
		};
	}

	void LoadProgram(std::vector<std::uint8_t> program)
	{
		instruction_memory = program;
		while (reg_pc >= 0 && reg_pc < instruction_memory.size()) { // If PC points to an instruction that doesn't exist, return
			if(!Decode()) {
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

