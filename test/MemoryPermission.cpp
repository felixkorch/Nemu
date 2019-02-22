// ---------------------------------------------------------------------* C++ *-
// MemoryPermission.cpp
//
// Checks that the objects of MemoryPermission behaves correctly.
// -----------------------------------------------------------------------------

#include "Nemu/MemoryPermission.h"
#include <cassert>
#include <cstdint>

int main(int argc, char **argv)
{
	using namespace nemu;
	using Flag = MemoryPermissionFlag;

	std::uint32_t value = 10;
	MemoryPermission<std::uint32_t> rw(Flag::RW, &value);

	std::uint8_t value8 = rw;
	assert(value8 == static_cast<std::uint8_t>(10));

	assert(rw == 10);
	rw = 11;
	assert(rw == 11);

	MemoryPermission<std::uint32_t> r(Flag::R, &value);
	assert(r == 11);
	r = 20;
	assert(r == 11);

	MemoryPermission<std::uint32_t> w(Flag::W, &value);
	assert(w == std::uint32_t());
	w = 13;
	assert(r == 13);

	return 0;
}