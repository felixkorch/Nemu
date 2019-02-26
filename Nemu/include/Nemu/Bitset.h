#pragma once
#include <cstdint>
#include <stdint.h>

template <int size>
class BitSet {

};

template <>
class BitSet<8> { // Template specialization for 8 bit
private:
	std::uint8_t mask;
public:
	std::uint8_t operator[](unsigned int index)
	{
		return IsSet(index);
	}

	void operator=(unsigned int val)
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
			throw std::out_of_range("Error: bitset out of range!");
		mask |= (1 << index);
	}

	void Set(unsigned int index, bool pred)
	{
		if (pred)
			Set(index);
		else
			Reset(index);
	}

	void Reset(unsigned int index)
	{
		if (index > 7)
			throw std::out_of_range("Error: bitset out of range!");
		mask &= ~(1 << index);
	}

	bool IsSet(unsigned int index)
	{
		if (index > 7)
			throw std::out_of_range("Error: bitset out of range!");
		return (mask & (1 << index)) != 0;
	}
};