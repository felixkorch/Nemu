// ---------------------------------------------------------------------* C++ *-
// NESMemory16Read.cpp
//
// Test UInt16 read from NESMemory.
// -----------------------------------------------------------------------------

#include "Nemu/NESMemory.h"
#include "Nemu/Mapper/NROM256Mapper.h"
#include <cassert>
#include <cstdint>
#include <vector>

// #include <iostream>
// #include <ios>
// #include <iomanip>

int main(int argc, char **argv)
{
	using namespace nemu;

	auto memory =
		MakeNESMemory<std::vector<std::uint32_t>, NROM256Mapper>();

	for (int i = 0x2000; i < 0x2008; ++i) {
		memory[i] = static_cast<std::uint8_t>(i);
	}

	// Dump memory

	// for (int i = 0; i < 0x10000; ++i) {
	// 	std::cout << std::hex << std::setfill('0') << "0x"
	// 		  << std::setw(4) << i << ": 0x" << std::setw(2)
	// 		  << memory[i] << '\n';
	// }

	for(int i = 0; i < 256; ++i) {
		assert(((((i + 1) & 0x0007) << 8) | (i & 0x0007)) == memory.Get16At(0x2000 + i));
	}

	return 0;
}
