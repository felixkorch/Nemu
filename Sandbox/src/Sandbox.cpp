#include "Nemu/CPU.h"
#include "Nemu/System.h"

using nemu::uint8;
using nemu::int8;
using nemu::uint16;

int main()
{
	std::vector<uint8> program = { // Subtraction(zpg[1] - zpg[0])
		0xA5, 1,      // lda  zpg[1]
		0x18,         // clc
		0xE5, 0,      // sbc  zpg[0]
		0x85, 2,      // sta  zpg[2]
	};

	std::vector<uint8> subroutine = {
		0xA2, 0xEE,    // ldx  0xEE     | Sub routine start
		0x86, 0x0A,    // stx  zpg[10]
		0x60,          //               | Return to (label A)


		0xA5, 0,       // lda  zpg[0]   | Entry point (0x8005)
		0x20, 0, 0x80, // JSR
		0x18,          // clc (label A)
		0x65, 1,       // adc  zpg[1]
		0x85, 2,	   // sta  zpg[2]
	};

	nemu::CPU cpu;
	std::vector<uint8>& ram = cpu.GetRAM();
	*(uint16*)(ram.data() + 0xFFFC) = 0x8005; // Set reset-vector to start address

	uint8 A = 200;   // A -> zpg[0]
	uint8 B = 100;   // B -> zpg[1]
	ram[0] = A;
	ram[1] = B;

	cpu.LoadProgram(subroutine, 0x8000);
	for (int i = 0; i < 15; i++) {
		std::cout << "[" << i << "] " << +(uint8)ram[i] << std::endl;
	}
}