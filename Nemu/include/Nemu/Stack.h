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
    /// The stack is of the type empty stack and uses a wrap-around mechanism.
    ///
    /// TODO: Implement Stack under/overflow.

    template <class Iterator>
    class Stack {
        using ValueType = typename Iterator::value_type;

	private:
        const Iterator lowerBound;
        const Iterator upperBound;
        Iterator it;

    public:
        constexpr Stack(const Iterator& lowerBound, const Iterator& upperBound)
                : lowerBound(lowerBound),
				  upperBound(upperBound),
                  it(upperBound) {}

        template <class Memory>
        constexpr Stack(Memory& memory, std::size_t size, std::size_t beginAddress = 0x0100)
                : Stack(std::next(memory.begin(), beginAddress),
					    std::next(memory.begin(), beginAddress + size - 1)) {}

        bool Push(const ValueType& data)
        {
			*it = data;
			if (it == lowerBound)
				it = upperBound;
			else
				--it;
            return true;
        }

        ValueType Pop()
        {
			if (it == upperBound) {
				it = lowerBound;
				return *it;
			}
            return *++it;
        }

        bool IsEmpty()
        {
            return it == upperBound;
        }

        std::uint8_t GetSP()
        {
            return std::distance(lowerBound, it);
        }

        void SetSP(std::uint8_t value)
        {
			it = lowerBound + value;
        }
    };

} // namespace nemu