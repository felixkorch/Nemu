// ---------------------------------------------------------------------* C++ *-
// EasyAdd.cpp
//
// Loads a program from static binary data that executes the add instruction
// with two positive integers that cannot overflow.
// -----------------------------------------------------------------------------

#include "Nemu/CPU.h"
#include <cassert>

constexpr static std::uint8_t N1 = 11;
constexpr static std::uint8_t N2 = 22;

int main(int argc, char **argv)
{
	nemu::CPU cpu;
	cpu.GetRAM()[0xFFFD] = 0x80;
	cpu.GetRAM()[0xFFFC] = 0;
	cpu.LoadProgram(
		{
			// clang-format off
			
			0xA9, N1,    // lda N1
			0x18,        // clc
			0x69, N2,    // adc N2
			0x8D, 0, 0,  // sta RESULTAT_LO

			// clang-format on
		},
		0x8000);

	assert(N1 + N2 == cpu.GetRAM()[0]);

	return 0;
}