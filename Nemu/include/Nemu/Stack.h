#pragma once
#include <vector>
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <iterator>

namespace nemu
{
	/// The stack acts as subset to a underlying container and uses a stack-like
	/// bevavior in order to manipulate it.
    template <class Iterator>
    class Stack {
        using ValueType = typename Iterator::value_type;

		// TODO:
		// 	beginAddress is currently unused other then in initialazation.
		//	Might not be needed as a parameter.
        const std::size_t beginAddress;
        const Iterator lowerBound;
        const Iterator upperBound;
        Iterator it;

    public:
        constexpr Stack(const Iterator &lowerBound,
                        const Iterator &upperBound,
                        std::size_t beginAddress = 0x0100)
                : lowerBound(lowerBound), upperBound(upperBound),
                  it(upperBound), beginAddress(beginAddress)
        {}

        template <class Memory>
        constexpr Stack(Memory &memory,
                        std::size_t size,
                        std::size_t beginAddress = 0x0100)
                : Stack(std::next(memory.begin(), beginAddress),
                        std::next(memory.begin(), beginAddress + size),
                        beginAddress)
        {}

        bool Push(const ValueType &data)
        {
            if (std::distance(it, lowerBound) < 0) {
                std::cout << "Stack Overflow" << std::endl;
                return false;
            }
            *it = data;
            --it;
            return true;
        }

        ValueType Pop()
        {
            if (IsEmpty()) {
                throw std::underflow_error("Nothing on stack");
            }
            return *++it;
        }

        bool IsEmpty()
        {
            return it == upperBound;
        }

        std::uint8_t GetSP()
        {
            return std::distance(it, upperBound);
        }

        void SetSP(std::uint8_t value)
        {
			std::advance(it, std::distance(lowerBound, it) + value);
        }
    };

} // namespace nemu