// ---------------------------------------------------------------------* C++ *-
// NROM256Mapper.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/Mapper/Mapper.h"
#include <cstddef>

namespace nemu
{
    /// Provides mapping layout for the NROM-256 cartridge.
    ///
    /// Mapping:
    ///     Block0:
    ///             range: (0x8000, 0xFFFF)
    ///             size: 0x8000 (32kB)
    ///
    struct NROM256MapperLayout {
        constexpr bool ContainsAddress(std::size_t address) const
        {
            return address >= 0x8000 && address <= 0xFFFF;
        }

        constexpr std::size_t AllocSize() const
        {
            return 0x8000;
        }

        template <class Iterator>
        constexpr Iterator Next(const Iterator &iterator, std::size_t address) const
        {
            return std::next(iterator, (address - 0x8000) % 0x8000);
        }
    };

    template <class Iterator>
    struct NROM256MapperBase : public Mapper<Iterator, NROM256MapperLayout> {
        NROM256MapperBase(const Iterator &iterator)
                : Mapper<Iterator, NROM256MapperLayout>(iterator, NROM256MapperLayout(), 0x8000)
        {}
    };

    struct NROM256Mapper {
        using Layout = NROM256MapperLayout;

        template <class Storage>
        using BaseMapper = NROM256MapperBase<typename Storage::iterator>;
    };

} // namespace nemu