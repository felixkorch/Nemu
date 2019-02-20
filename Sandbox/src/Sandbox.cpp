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

	std::vector<uint8> subroutine = {
		0x85, 0xEE,   // sta  zpg[10] (sub routine)
		0x60,         // Return to (label A)
		0xA5, 1,      // lda  zpg[1]
		0x20, 0x80, 0,// JSR
		0x18,         // clc (label A)
		0xE5, 0,      // sbc  zpg[0]
		0x85, 2,	  // sta  zpg[2]

	};

	nemu::CPU cpu;
	std::vector<uint8>& ram = cpu.GetRAM();

	uint8 B = -80; // B -> zpg[1]
	uint8 A = -5;  // A -> zpg[0]
	ram[0] = A;
	ram[1] = B;

	cpu.LoadProgram(subroutine, 0x8002);
	for (int i = 0; i < 15; i++) {
		std::cout << +(int8)ram[i] << std::endl;
	}

	std::cin.get();
}