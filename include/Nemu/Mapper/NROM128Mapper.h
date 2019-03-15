// ---------------------------------------------------------------------* C++ *-
// NROM128Mapper.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/Mapper/Mapper.h"
#include <cstddef>

namespace nemu
{
    /// Provides mapping for the NROM-128 cartridge layout.
    ///
    /// Mapping:
    ///     Block0:
    ///             range: (0x8000, 0xFFFF)
    ///             size: 0x8000 (16kB)
    ///             modulus: 0x4000
    ///
    struct NROM128MapperLayout {
        constexpr bool ContainsAddress(std::size_t address) const
        {
            return address >= 0x8000 && address <= 0xFFFF;
        }

        constexpr std::size_t AllocSize() const
        {
            return 0x4000;
        }

        template <class Iterator>
        constexpr Iterator Next(const Iterator &iterator, std::size_t address) const
        {
            return std::next(iterator, (address - 0x8000) % 0x4000);
        }
    };

    template <class Iterator>
    struct NROM128MapperBase : public Mapper<Iterator, NROM128MapperLayout> {
        NROM128MapperBase(const Iterator &iterator)
                : Mapper<Iterator, NROM128MapperLayout>(iterator, NROM128MapperLayout(), 0x8000)
        {}
    };

    struct NROM128Mapper {
        using Layout = NROM128MapperLayout;

        template <class Storage>
        using BaseMapper = NROM128MapperBase<typename Storage::iterator>;
    };
} // namespace nemu