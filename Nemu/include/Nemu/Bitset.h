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
	public:

		Bitset()
			: mask(0) {}

		Bitset(std::uint8_t val)
			: mask(val) {}

		template <typename... Args, typename = typename std::enable_if<sizeof...(Args) == 8>::type>
		Bitset(Args&&... args)
		{
		}

		bool operator[](unsigned int index)
		{
			return IsSet(index);
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
			if (index > 7)
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
			if (index > 7)
				throw std::out_of_range("Error: Bitset out of range!");
			mask &= ~(1 << index);
		}

		bool IsSet(unsigned int index)
		{
			if (index > 7)
				throw std::out_of_range("Error: Bitset out of range!");
			return (mask & (1 << index)) != 0;
		}

		friend std::ostream& operator<<(std::ostream &out, Bitset<8>& set)
		{
			for (int i = 0; i < 8; i++) {
				out << +set.IsSet(i);
			}
			return out;
		}
	};
}