// ---------------------------------------------------------------------* C++ *-
// MaskIteratorRead.cpp
//
// Checks so that the mask iterator maps correct values.
// -----------------------------------------------------------------------------

#include "Nemu/Mapper/MaskIterator.h"
#include <vector>
#include <cassert>
#include <utility>
#include <iterator>

int main(int argc, char **argv)
{
	std::vector<int> v;
	for (int i = 0; i < 256; ++i)
		v.push_back(i);

	auto begin = std::next(v.begin(), 16);
	nemu::MaskIterator<decltype(begin)> it(begin, 0xFF);

	for (int i = 0; i < 64; i++) {
		assert(*std::next(begin, (i & 0xFF)) == *std::next(it, i));
	}

	for (int i = 0; i < 64; i++) {
		assert(*std::next(begin, (i & 0xFF)) == *it);
		++it;
	}

	return 0;
}