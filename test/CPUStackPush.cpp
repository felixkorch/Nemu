#include "Nemu/CPU.h"
#include "Nemu/NESMemory.h"
#include "Nemu/Mapper/NROM256Mapper.h"
#include <algorithm>
#include <cassert>
#include <cstdint>

using namespace nemu;

int main()
{
    // TOTO:
    //  Write program as binary vector.

    // _init:
    //   ldx #$ff	; Set the stack pointer to $FF
    //   txs		; (e.g. $01FF)
    //
    // _pushstack:
    //   lda #$e0	; Push value $e0 on to the stack.
    //   pha		; $01FF now contains $e0, and S is now $FE.
    //
    //   ldy #$bb	; Push value $bb on to the stack.
    //   tya
    //   pha		; $01FE now contains $bb, and S is now $FD.
    //
    //   txa
    //   pha		; Push value $ff (from the _init routine) on to the stack.
    // 		        ; $01FD now contains $ff, and S is now $FC.

    auto memory = MakeNESMemory<std::vector<std::uint32_t>, NROM256Mapper>();
    CPU<decltype(memory)> cpu(memory);

    std::copy(program.begin(),
              program.end(),
              std::next(memory.begin(), 0x8000));

    // Set reset-vector to start address
    memory[0xFFFC] = 0x00;
    memory[0xFFFD] = 0x80;

    uint8 A = 200; // A -> zpg[0]
    memory[0] = A;

    cpu.Run();
    cpu.Clock(10);

    // Assetion...

    return 0;
}