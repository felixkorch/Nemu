// ---------------------------------------------------------------------* C++ *-
// NROM128ReadWrite.cpp
//
// -----------------------------------------------------------------------------

#include "Nemu/Mapper/NROM128Mapper.h"
#include <vector>
#include <cassert>
#include <utility>
#include <iterator>

int main(int argc, char **argv)
{
    using Storage = std::vector<int>;
    using MapperBase = nemu::NROM128MapperBase<std::vector<int>::iterator>;

    auto vector = Storage(nemu::NROM128MapperLayout().AllocSize());
    MapperBase mapper(vector.begin());

    *mapper = 0xCC;
    assert(*mapper == 0xCC);
    assert(*std::next(mapper, 0xC000) == 0xCC);

    return 0;
}