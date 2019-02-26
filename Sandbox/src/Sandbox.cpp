#include "Nemu/CPU.h"
#include "Nemu/System.h"
#include <cassert>

using namespace nemu;

int main()
{
	std::vector<std::uint8_t> program = { // Subtraction(zpg[1] - zpg[0])
		0xA5, 1,      // lda  zpg[1]
		0x18,         // clc
		0x65, 0,      // adc  zpg[0]
		0x85, 2,      // sta  zpg[2]
		0xA2, 1,      // ldx  #1
		0xD0, 0       // while 1
	};

	std::vector<std::uint8_t> subroutine = {
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

	std::vector<std::uint8_t> easy_and = {
		0xA5, 0,       // lda  zpg[0]        | Entry point (0x8000)
		0x29, 0xFF,    // and
		0x85, 1,       // sta  zpg[0] & 0xFF -> zpg[1]
		0xA2, 1,       // ldx  #1
		0xD0, 0        // while 1
	};

		
	auto nestest = ReadFile<std::vector<std::uint8_t>>("C:/dev/VS/Nemu/Sandbox/programs/6502_functional_test.bin");
	std::cout << "Size of nestest: " << nestest.size() << std::endl;
	VectorMemory<std::uint8_t> memory(nestest);

	memory[0xFFFC] = 0x00;   // Load reset vector with start address
	memory[0xFFFD] = 0x04;   // 05d4

	CPU<decltype(memory)> cpu(memory);

	while (true) {
		cpu.Clock(1);
		cpu.PrintRegisters();
		cpu.PrintFlags();
	}
}