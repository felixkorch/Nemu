// ---------------------------------------------------------------------* C++ *-
// CPUStackPush.cpp
//
// Test basic stack operations.
// -----------------------------------------------------------------------------


#include "Nemu/CPU.h"
#include "Nemu/NESMemory.h"
#include "Nemu/Mapper/NROM256Mapper.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>

// #include <iostream>
// #include <ios>
// #include <iomanip>

using namespace nemu;

int main()
{
    // clang-format off
    std::vector<std::uint32_t> program{
        // init:
        0xA2, 0xFF, // ldx #$ff	; Set the stack pointer to $FF
        0x9A,       // txs		; (e.g. $01FF)
        // push_from_a: ; clock(2)
        0xA9, 0xCC, // lda #$cc	; Push value $cc on to the stack.
        0x48,       // pha		; $01FF now contains $cc, and S is now $FE.
        // push_from_y: ; clock(4)
        0xA0, 0xBB, // ldy #$bb	; Push value $bb on to the stack.
        0x98,       // tya
        0x48,       // pha		; $01FE now contains $bb, and S is now $FD.
        // push_from_x; ; clock(7)
        0xA2, 0xAA, // ldx #$aa
        0x8A,       // txa
        0x48,       // pha		; Push value $aa (from the _init routine) on
                    //          ; to the stack.
                    //          ; $01FD now contains $aa, and S is now $FC.
        // pop_to_a:    ; clock(10)
        0x68,       // pla      ; Pop $aa to register A.
        // push_from_a: ; clock(11)
        0xA9, 0x99, // lda #$99 ;
        0x48,       // pha      ; Push $99 to address $01fd
        // end:         ; clock(13)
    };
    // clang-format on

    auto memory = MakeNESMemory<std::vector<std::uint32_t>, NROM256Mapper>();
    CPU<decltype(memory)> cpu(memory);

    std::copy(
            program.begin(), program.end(), std::next(memory.begin(), 0x8000));

    // Set reset-vector to start address
    memory[0xFFFC] = 0x00;
    memory[0xFFFD] = 0x80;

    uint8 A = 200; // A -> zpg[0]
    memory[0] = A;

    cpu.Run();

    cpu.Clock(10); // cycle 0 -> 10

    // Dump memory

	// for (int i = 0; i < 0x10000; ++i) {
	// 	std::cout << std::hex << std::setfill('0') << "0x"
	// 		  << std::setw(4) << i << ": 0x" << std::setw(4)
	// 		  << memory[i] << '\n';
	// }

    assert(memory[0x01FF] == 0xCC);
    assert(memory[0x01FE] == 0xBB);
    assert(memory[0x01FD] == 0xAA);

    cpu.Clock(1); // cycle 10 -> 11
    assert(cpu.GetRegisterA() == 0xAA);

    cpu.Clock(2); // cycle 11 -> 13
    assert(memory[0x01FD] == 0x99);

    return 0;
}