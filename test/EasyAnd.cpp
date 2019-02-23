#include "Nemu/CPU.h"
#include "Nemu/NESMemory.h"
#include "Nemu/Mapper/NROM256Mapper.h"
#include <algorithm>
#include <cassert>
#include <cstdint>

using namespace nemu;

int main()
{
    // clang-format off
    std::vector<uint8> easy_and = {
        0xA5, 0,    // lda  zpg[0]        | Entry point (0x8000)
        0x29, 0xFF, // and
        0x85, 1,    // sta  zpg[0] & 0xFF -> zpg[1]
        0xA2, 1,    // ldx  #1
        0xD0, 0     // while 1
    };
    // clang-format on

    auto memory = MakeNESMemory<std::vector<std::uint32_t>, NROM256Mapper>();
    CPU<decltype(memory)> cpu(memory);

    std::copy(easy_and.begin(),
              easy_and.end(),
              std::next(memory.begin(), 0x8000));

    // Set reset-vector to start address
    memory[0xFFFC] = 0x00;
    memory[0xFFFD] = 0x80;

    uint8 A = 200; // A -> zpg[0]
    memory[0] = A;

    cpu.Run();
    cpu.Clock(10);

    // cpu.PrintMemory<uint8>(5);
    assert(memory[1] == (A & 0xFF));
}