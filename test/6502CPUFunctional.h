// ---------------------------------------------------------------------* C++ *-
// 6502CPUFunctional.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/CPU.h"
#include "Nemu/System.h"
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace nemu {
namespace test {

class Memory {
    std::vector<uint8_t> data;

  public:
    Memory() : data(ReadFile<std::vector<std::uint8_t>>(NEMU_6502CPUFUNCTIONAL_ASMFILE))
    {
        data[0xFFFC] = 0x00;
        data[0xFFFD] = 0x04;
    }

    std::uint8_t Read(std::size_t address)
    { return data[address]; }

    void Write(std::size_t address, std::uint8_t value)
    { data[address] = value; }
};

class TestBase {
  public:
    void Run() {
        CPU<Memory> cpu(std::make_shared<Memory>());
        cpu.Tick = []() {};
        cpu.Reset();

        auto timeout = std::chrono::system_clock::now() + std::chrono::seconds(5);

        // TODO: 
        //  End test when completed. Timeout should only accure if something fails.
        while (true) {
            assert(std::chrono::system_clock::now() < timeout);
            cpu.DecrementCycles();
        }
    }
};

} // namespace test
} // namespace nemu