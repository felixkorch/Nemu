// ---------------------------------------------------------------------* C++ *-
// NESMemory16Read.cpp
//
// Test UInt16 read from NESMemory.
// -----------------------------------------------------------------------------

#include "Nemu/NESMemory.h"
#include "Nemu/Mapper/NROM256Mapper.h"
#include <cassert>
#include <cstdint>
#include <vector>

int main(int argc, char **argv)
{
    using namespace nemu;

    NESMemory<std::vector<std::uint32_t>, NROM256Mapper> memory;

    for (int i = 0x2000; i < 0x2008; ++i) {
        memory[i] = static_cast<std::uint8_t>(i);
    }

    for (int i = 0; i < 256; ++i) {
        assert(((((i + 1) & 0x0007) << 8) | (i & 0x0007)) == memory.Get16At(0x2000 + i));
    }

    return 0;
}
