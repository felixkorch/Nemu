// ---------------------------------------------------------------------* C++ *-
// MaskIteratorWrite.cpp
//
// Checks so that the mask iterator maps correct values.
// -----------------------------------------------------------------------------

#include "Nemu/Mapper/MaskIterator.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <vector>
#include <iostream>

int main(int argc, char **argv)
{
	std::vector<int> v(32, 0);
	nemu::MaskIterator<decltype(v.begin())> it(std::next(v.begin(), 8),
						   0x07);

	int n = 0;
	std::generate(it, it + 32, [&n] { return ++n; });

	// clang-format off
	std::vector<int> u{ 
         	 0,  0,  0,  0,  0,  0,  0,  0, 
        	25, 26, 27, 28, 29, 30, 31, 32, 
         	 0,  0,  0,  0,  0,  0,  0,  0,  
         	 0,  0,  0,  0,  0,  0,  0,  0,
    	};
	// clang-format on

	assert(std::equal(v.cbegin(), v.cend(), u.cbegin()));

	return 0;
}