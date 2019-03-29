#include "Nemu/CPU.h"
#include "Nemu/System.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <cstdint>

using namespace nemu;

class BaseMemoryType {
protected:
	unsigned data;
public:
	BaseMemoryType(unsigned value)
		: data(value) {}
	virtual BaseMemoryType& operator=(unsigned value) = 0;
	virtual operator unsigned() const = 0;
};

class StandardMemoryType : public BaseMemoryType {
public:
	StandardMemoryType(unsigned value)
		: BaseMemoryType(value) {}

	virtual BaseMemoryType& operator=(unsigned value) override
	{
		this->data = value;
		return *this;
	}

	virtual operator unsigned() const override
	{
		return this->data;
	}
};

int main()
{
	auto nestest = ReadFile<std::vector<std::uint8_t>>("C:/dev/VS/Nemu/test/asm/6502_functional_test.bin");
	std::cout << "Size of nestest: " << nestest.size() << std::endl;
	std::vector<unsigned> temp;
	for (auto& a : nestest)
		temp.push_back(a);
	VectorMemory<unsigned> memory(temp);

	memory[0xFFFC] = 0x00;   // Load reset vector with start address (0x0400)
	memory[0xFFFD] = 0x04;

	CPU<decltype(memory), unsigned> cpu(memory);

	auto begin = std::chrono::steady_clock::now();
	while (true) {
		cpu.Execute();
	}
	auto end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(end - begin).count() << std::endl;
}