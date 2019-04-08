// ---------------------------------------------------------------------* C++ *-
// Test.cpp
//
// -----------------------------------------------------------------------------

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
    Test test;

    std::cout << "Start test " << TestName(NEMU_TEST_FILE) << "\t\n\t" << std::flush;
    test.Run();
    std::cout << "test finished successfully\n";

    return 0;
}

