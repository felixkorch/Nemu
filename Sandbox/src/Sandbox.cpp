#include "Nemu/CPU.h"
#include "Nemu/System.h"

using namespace nemu;

int main()
{
	std::vector<uint8> program = { // Subtraction(zpg[1] - zpg[0])
		0xA5, 1,      // lda  zpg[1]
		0x18,         // clc
		0x65, 0,      // adc  zpg[0]
		0x85, 2,      // sta  zpg[2]
		0xA2, 1,      // ldx  #1
		0xD0, 0       // while 1
	};

	std::vector<uint8> subroutine = {
		0xA2, 0xFF,    // ldx  0xFF     | Sub routine start
		0x86, 0x0A,    // stx  zpg[10]  
		0x60,          //               | Return to (label A)



		0xA5, 0,       // lda  zpg[0]   | Entry point (0x8005)
		0xC9, 200,     // CMP Immediate #200
		0xD0, 5,       // BNE +5
		0x20, 0, 0x80, // JSR jump to sub
		0x18,          // clc (label A)
		0x65, 1,       // adc  zpg[1]
		0x85, 2,	   // sta  zpg[2]
		0xA2, 1,       // ldx  #1
		0xD0, 0        // while 1
	};

	CPU<std::vector<uint8>> cpu;
	auto& memory = cpu.GetMemory();
	std::copy(subroutine.begin(), subroutine.end(), &memory[0] + 0x8000);

	AsUInt16(0xFFFC) = 0x8005; // Set reset-vector to start address

	uint8 A = 200;   // A -> zpg[0]
	uint8 B = 40;    // B -> zpg[1]
	memory[0] = A;
	memory[1] = B;

	cpu.Run();
	cpu.Clock(10);
	cpu.PrintFlags();
	cpu.PrintRegisters();
	cpu.PrintMemory<uint8>(15);
	std::cin.get();
}