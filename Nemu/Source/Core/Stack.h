#pragma once

template <int size>
class Stack {
private:
	std::uint8_t memory[size];
	int sp;
public:

	Stack()
		: sp(-1) {}

	bool Push(std::uint8_t byte)
	{
		if (sp >= (size - 1)) {
			std::cout << "Stack Overflow" << std::endl;
			return false;
		}
		memory[++sp] = byte;
		return true;
	}

	std::uint8_t Pop()
	{
		if (sp < 0) {
			std::cout << "Nothing on stack" << std::endl;
			return;
		}
		return memory[sp--];
	}

	bool isEmpty()
	{
		return (sp < 0);
	}
};

