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

template <class T>
class Test: T {
    T* Super() { return static_cast<T*>(this); }
  public:
    void Run() { Super()->Run(); }
};

} // namespace test
} // namespace nemu

int main(int argc, char** argv)
{
    nemu::test::Test<nemu::test::TestBase> test;

    std::cout << "Start test from \"" << NEMU_TEST_FILE << "\"\t\n\t" << std::flush;
    test.Run();
    std::cout << "> test finished successfully\n";

    return 0;
}

