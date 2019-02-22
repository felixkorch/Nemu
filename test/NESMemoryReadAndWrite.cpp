// ---------------------------------------------------------------------* C++ *-
// NESMemoryReadAndWrite.cpp
//
// Tests the basic RW of the NES memory.
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
	using Storage = std::vector<std::uint32_t>;

	auto memory = MakeNESMemory<Storage, NROM256Mapper<Storage::iterator>>(
		Storage(NROM256MemorySize()));

	for (int i = 0; i < 0x10000; ++i) {
		memory[i] = i + 1;
		assert(memory[i] == i + 1);
	}

	// Dump memory
	// for (int i = 0; i < 0x10000; ++i) {
	//	std::cout << std::hex << std::setfill('0') << "0x"
	//		  << std::setw(4) << i << ": 0x" << std::setw(4)
	//		  << memory[i] << '\n';
	// }

	return 0;
}
