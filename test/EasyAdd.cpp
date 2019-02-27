// ---------------------------------------------------------------------* C++ *-
// EasyAdd.cpp
//
// Loads a program from static binary data that executes the add instruction
// with two positive integers that cannot overflow.
// -----------------------------------------------------------------------------

#include "Nemu/CPU.h"
#include <cassert>

using namespace nemu;

int main(int argc, char **argv)
{
	std::uint8_t N1 = std::stoi(argv[1]);
	std::uint8_t N2 = std::stoi(argv[2]);

	VectorMemory<std::uint8_t> memory;
	memory[0xFFFD] = 0x80;
	memory[0xFFFC] = 0x00;

	std::vector<std::uint8_t> p
	{
		// clang-format off
			
		0xA9, N1,    // lda N1
		0x18,        // clc
		0x69, N2,    // adc N2
		0x85, 0,     // sta RESULTAT

		// clang-format on
	};

	std::copy(p.begin(), p.end(), memory.data() + 0x8000);

	CPU<decltype(memory)> cpu(memory);
	cpu.Execute(4);

	cpu.PrintMemory<std::uint8_t>(10);
	cpu.PrintFlags();

	std::uint8_t ans = N1 + N2;
	assert(ans == cpu.GetMemory()[0]);

	return 0;
}