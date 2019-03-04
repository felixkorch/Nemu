#pragma once
#include <cstdint>
#include <stdint.h>
#include <iostream>
#include <array>

namespace nemu
{
	template <int size>
	class Bitset {
	};

	template <>
	class Bitset<8> { // Template specialization for 8 bit
	private:
		std::uint8_t mask;
		static constexpr unsigned int Size = 8;
	public:

		Bitset()
			: mask(0) {}

		Bitset(std::uint8_t val)
			: mask(val) {}

		bool operator[](unsigned int index)
		{
			return At(index);
		}

		void operator=(std::uint8_t val)
		{
			mask = val;
		}

		operator std::uint8_t() const
		{
			return mask;
		}

		void Set(unsigned int index)
		{
			if (index > Size - 1)
				throw std::out_of_range("Error: Bitset out of range!");
			mask |= (1 << index);
		}


		void Set(unsigned int index, bool pred)
		{
			if (pred)
				Set(index);
			else
				Clear(index);
		}

		void Clear(unsigned int index)
		{
			if (index > Size - 1)
				throw std::out_of_range("Error: Bitset out of range!");
			mask &= ~(1 << index);
		}

		bool At(unsigned int index)
		{
			if (index > Size - 1)
				throw std::out_of_range("Error: Bitset out of range!");
			return (mask & (1 << index)) != 0;
		}

		friend std::ostream& operator<<(std::ostream &out, Bitset<8>& set)
		{
			for (int i = 0; i < 8; i++) {
				out << +set.At(i);
			}
			return out;
		}
	};
}