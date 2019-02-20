#include "Nemu/CPU.h"
#include "Nemu/System.h"

using nemu::uint8;
using nemu::int8;

int main()
{
	std::vector<uint8> program = { // Subtraction(zpg[1] - zpg[0])
		0xA5, 1,      // lda  zpg[1]
		0x18,         // clc
		0xE5, 0,      // sbc  zpg[0]
		0x85, 2,      // sta  zpg[2]
	};

	nemu::CPU cpu;
	std::vector<uint8>& ram = cpu.GetRAM();

	uint8 B = -80; // B -> zpg[1]
	uint8 A = -5;  // A -> zpg[0]
	ram[0] = A;
	ram[1] = B;

	cpu.LoadProgram(program, 0x8000);
	std::cout << "B - A = " << +(int8)ram[2] << std::endl;

	std::cin.get();
}