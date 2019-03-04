#include "Nemu/FlagRegister.h"
#include <cassert>

int main(int argc, char **argv) 
{
    nemu::FlagRegister reg;

    reg.c = 1;
    reg.i = 1;
    assert(reg == 5);

    reg.c = 0;
    assert(reg == 4);
    assert(reg.c == 0);
    assert(reg.i == 1);

    reg = 0x40;
    assert(reg.v == 1);
    assert(reg.b == 0);
    assert(reg = 0x40);

    return 0;
}