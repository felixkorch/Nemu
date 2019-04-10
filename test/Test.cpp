// -----------------------------------------------------------------------------------------* C++ *-
// Test.cpp
//
// -------------------------------------------------------------------------------------------------

#include <chrono>
#include <iostream>

#ifndef NEMU_TEST_FILE
#define NEMU_TEST_FILE "Dummy.h"
#endif

#include NEMU_TEST_FILE

namespace nemu {
namespace test {

std::string TestName(const std::string& fileName)
{ return fileName.substr(0, fileName.find(".h")); }

} // namespace test
} // namespace nemu

using namespace nemu::test;

int main(int argc, char** argv)
{
    using Clock = std::chrono::steady_clock;
    using Seconds = std::chrono::duration<double, std::ratio<1, 1>>;

    Test test;

    std::cout << "Start test " << TestName(NEMU_TEST_FILE) << "\t\n\t" << std::flush;

    auto beginTime = Clock::now();
    test.Run();
    auto duration = std::chrono::duration_cast<Seconds>(Clock::now() - beginTime).count();

    std::cout << "test finished successfully in " << duration << " seconds\n";

    return 0;
}

