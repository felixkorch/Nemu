#pragma once
#include <vector>
#include <iostream>
#include <cstdint>

namespace nemu {

	template <class T, unsigned int size>
	class Stack {
	private:
		unsigned int sp;
		T* memory;
	public:

		Stack(T* mem)
			: sp(size), memory(mem) {}

		bool Push(T data)
		{
			if (sp == 0) {
				std::cout << "Stack Overflow" << std::endl;
				return false;
			}
			*memory-- = data;
			sp--;
			return true;
		}

		T Pop()
		{
			if (sp == size - 1) {
				throw std::exception("Nothing on stack");
			}
			sp++;
			return *memory++;
		}

		bool isEmpty()
		{
			return (sp == 0);
		}

		std::uint8_t GetSP()
		{
			return sp & 0xFF;
		}
	};

}