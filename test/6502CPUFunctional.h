// -----------------------------------------------------------------------------------------* C++ *-
// 6502CPUFunctional.h
//
// -------------------------------------------------------------------------------------------------

#pragma once

#include "Nemu/CPU.h"
#include "Nemu/System.h"
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

//
// (!) CURRENTLY DOES NOT PASS (!)
//

namespace nemu {
namespace test {

class Memory {
    std::vector<uint8_t> data;

  public:
    Memory() : data(ReadFile<std::vector<std::uint8_t>>(NEMU_6502CPUFUNCTIONAL_ASMFILE))
    {
        // Set reset vector.
        data[0xFFFC] = 0x00;
        data[0xFFFD] = 0x04;
    }

    std::uint8_t Read(std::size_t address)
    { return data[address]; }

    void Write(std::size_t address, std::uint8_t value)
    { data[address] = value; }
};

class Test {
  public:
    void Run() {
        CPU<Memory> cpu(std::make_shared<Memory>());
        cpu.Tick = []() {};

        auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        cpu.Reset();

        int i = 0;
        while (true) {
            // Check for timeout every 4096 instruction to speed up test.
            if (++i == 4096) {
                assert(std::chrono::steady_clock::now() < timeout);
                i = 0;
            }

            cpu.StepInstruction();

            switch (cpu.regPC) {
            // Skip decimal add/subtract test.
            case 0x336D: cpu.regPC = 0x3405; break;
            // Skip decimal/binary switch test.
            case 0x3411: cpu.regPC = 0x345D; break;
            // Success address reached!
            case 0x3469: return;

            default: break;
            }
        }
    }
};

} // namespace test
} // namespace nemu