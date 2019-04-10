// -----------------------------------------------------------------------------------------* C++ *-
// NROM128ReadWrite.h
//
// -------------------------------------------------------------------------------------------------

#pragma once

#include "Nemu/Mapper/NROM128Mapper.h"
#include <cassert>
#include <vector>

namespace nemu {
namespace test {

class Test {
  public:
    void Run()
    {
        std::vector<unsigned> data(0x4000);
        data[0] = 3;
        data[1] = 5;
        for (unsigned i = 2; i < 0x4000; ++i) {
            data[i] = data[i - 2] + data[i - 1];
        }

        mapper::NROM128Mapper mapper(data.cbegin(), data.cend());

        for (unsigned i = 0; i <= 0xFFFF; ++i) {            
            if (i <= 0x7FFF)
                assert(mapper.Read(i) == 0);
            else if (i <= 0xBFFF)
                assert(mapper.Read(i) == (data[i - 0x8000] & 0xFF));
            else
                assert(mapper.Read(i) == (data[i - 0xC000] & 0xFF));
        }
    }
};

} // namespace test
} // namespace nemu
