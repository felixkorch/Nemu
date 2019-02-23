#pragma once
#include <vector>
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <iterator>

namespace nemu
{
	template <class Iterator, unsigned int size>
	class Stack {
		using ValueType = typename Iterator::value_type;

		unsigned int sp;
		Iterator it;

	    public:
		Stack(Iterator it) : sp(size), it(it)
		{}

		bool Push(const ValueType& data)
		{
			if (sp == 0) {
				std::cout << "Stack Overflow" << std::endl;
				return false;
			}
			*it = data;
			--it;
			--sp;
			return true;
		}

		ValueType Pop()
		{
			if (sp == size) {
				throw std::underflow_error("Nothing on stack");
			}
			++sp;
			return *++it;
		}

		bool isEmpty()
		{
			return (sp == 0);
		}

		std::uint8_t GetSP()
		{
			return sp & 0xFF;
		}

		void SetSP(std::uint8_t val)
		{
			std::advance(it, val - sp);
			sp = val;
		}
	};

} // namespace nemu