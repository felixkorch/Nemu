#include "Nemu/CPU/CPU.h"
#include "Nemu/System.h"
#include "Nemu/Nemu.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <cstdint>


using namespace nemu;

int main()
{
	/*auto nestest = ReadFile<std::vector<std::uint8_t>>("C:/dev/VS/Nemu/test/asm/6502_functional_test.bin");
	std::cout << "Size of nestest: " << nestest.size() << std::endl;
	std::vector<unsigned> memory;
	for (auto a : nestest)
		memory.push_back(a);

	memory[0xFFFC] = 0x00;   // Load reset vector with start address (0x0400)
	memory[0xFFFD] = 0x04;

	CPU cpu(memory);
	cpu.Reset();

	auto begin = std::chrono::steady_clock::now();
	while (true) {
		cpu.Execute();
	}
	auto end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(end - begin).count() << std::endl;*/
}