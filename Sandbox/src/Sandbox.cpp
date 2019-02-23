#include "Nemu/CPU.h"
#include "Nemu/System.h"
#include <cassert>

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

	std::vector<uint8> easy_and = {
		0xA5, 0,       // lda  zpg[0]        | Entry point (0x8000)
		0x29, 0xFF,    // and
		0x85, 1,       // sta  zpg[0] & 0xFF -> zpg[1]
		0xA2, 1,       // ldx  #1
		0xD0, 0        // while 1
	};

	
	auto nestest = ReadFile<std::vector<uint8>>("nestest.nes");
	
	auto memory = MakeNESMemory<std::vector<int32>, NROM256Mapper>();
	for (int i = 0x8000; i < nestest.size(); i++) {
		memory[i] = nestest[i];
	}
	CPU<decltype(memory)> cpu(memory);

	memory[0xFFFC] = 0;   // Load reset vector with start address
	memory[0xFFFD] = 0xC0;

	uint8 A = 200; // A -> zpg[0]
	memory[0] = A;

	cpu.Run();
	while (1) {
		cpu.Clock(1);
	}


	/*auto memory = std::vector<uint8>(program);
	CPU<std::vector<uint8>> cpu(memory);
	memory[0xFFFC] = 0;   // Load reset vector with start address
	memory[0xFFFD] = 0x80;
	uint8 A = 200; // A -> zpg[0]
	uint8 B = 25;  // B -> zpg[1]
	memory[0] = A;
	memory[1] = B;
	cpu.Run();
	cpu.Clock(10);
	cpu.PrintMemory<uint8>(10);*/
}