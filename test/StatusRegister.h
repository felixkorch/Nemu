// ---------------------------------------------------------------------* C++ *-
// StatusRegister.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/StatusRegister.h"
#include <cassert>

namespace nemu {
namespace test {

class Test {
  public:
    void Run() {
        nemu::StatusRegister reg;

        reg.C = 1;
        reg.I = 1;
        assert(reg == 5);

        reg.C = 0;
        assert(reg == 4);
        assert(reg.C == 0);
        assert(reg.I == 1);

        reg = 0x40;
        assert(reg.V == 1);
        assert(reg.B == 0);
        assert(reg = 0x40);
    }
};

} // namespace test
} // namespace nemu
