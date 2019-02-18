#include "Nemu/CPU.h"
#include "Nemu/System.h"
#include <iostream>
//#include "SFML/Window.hpp"

int main()
{
	std::uint16_t A = 20;
	std::uint16_t B = 30;
	std::uint8_t A_LOW = A & 0xff;
	std::uint8_t A_HIGH = A >> 8;
	std::uint8_t B_LOW = B & 0xff;
	std::uint8_t B_HIGH = B >> 8;

	std::vector<std::uint8_t> program = {
		0xA9, A_LOW,  // lda TAL1_LO
		0x18,         // clc
		0x69, B_LOW,  // adc TAL2_LO
		0x8D, 0, 0,   // sta RESULTAT_LO
		0xA9, A_HIGH, // lda TAL1_HI
		0x69, B_HIGH, // adc TAL2_HI
		0x8D, 1, 0    // sta RESULTAT_HI
	};

	std::vector<std::uint8_t> program2 = { // Loop
		0xA2, 10,			//0  LDX
		0x8A,				//2  TXA
		0x9D, 0, 0,		    //3  STA, X
		0xCA,				//6  DEX
		0xF0, 5,			//7  BEQ
		0x4C, 0x02, 0x80    //9  JMP
							//12 END
	};

	nemu::WriteFile("add.bin", (char*)program.data(), program.size());
	nemu::WriteFile("loop.bin", (char*)program2.data(), program.size());

	/*

	CPU cpu;
	cpu.LoadProgram(program, 0x8000); // 0x8000 ROM?

	auto ram = cpu.GetRAM();
	std::cout << A << " + " << B << " = " << *(std::uint16_t*)ram.data() << std::endl;
	/*
	for (unsigned int i = 0; i < 10; i++) {
		std::cout << +ram[i] << std::endl;
	} */

	std::cin.get();
}