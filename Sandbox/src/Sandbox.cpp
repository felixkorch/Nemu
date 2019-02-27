#include "Nemu/CPU.h"
#include "Nemu/System.h"
#include <cassert>
#include <chrono>
#include <bitset>

using namespace nemu;


int main()
{
	auto nestest = ReadFile<std::vector<std::uint8_t>>("C:/dev/VS/Nemu/test/asm/6502_functional_test.bin");
	std::cout << "Size of nestest: " << nestest.size() << std::endl;
	VectorMemory<std::uint8_t> memory(nestest);

	memory[0xFFFC] = 0x00;   // Load reset vector with start address (0x0400)
	memory[0xFFFD] = 0x04;

	CPU<decltype(memory)> cpu(memory);


	while (true) {
		cpu.Execute(1);
	}
}