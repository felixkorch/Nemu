// ---------------------------------------------------------------------* C++ *-
// InternalNESMapper.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/Mapper/Mapper.h"
#include <cstddef>

namespace nemu
{
    /// Mapping of the non cartrigde memory:
    /// Source: 'https://wiki.nesdev.com/w/index.php/CPU_memory_map'
    ///
    /// Internal RAM:
    ///     range: (0x0000, 0x1FFF)
    ///     size: 0x0800 (2048 B)
    ///     mask: 0x07FF
    ///     offset: 0x0000
    ///
    /// PPU registers:
    ///     range: (0x2000, 0x3FFF)
    ///     size: 0x0008 (8 B)
    ///     mask: 0x0007
    ///     offset: 0x0800
    ///
    /// NES APU and I/O registers:
    ///     range: (0x4000, 0x401F)
    ///     size: 0x0020 (36 B)
    ///     offset: 0x0808
    ///
    struct InternalNESMapperLayout {
        constexpr bool ContainsAddress(std::size_t address) const
        {
            return address < 0x4020;
        }

        constexpr std::size_t AllocSize() const
        {
            return 0x0828;
        }

        template <class Iterator>
        Iterator Next(const Iterator &iterator, std::size_t address) const
        {
            if (address < 0x2000)
                return std::next(iterator, address & 0x07FF);
            if (address < 0x4000)
                return std::next(std::next(iterator, 0x0800), address & 0x0007);
            return std::next(std::next(iterator, 0x0808), (address - 0x4000));
        }
    };

    template <class Iterator>
    struct InternalNESMapperBase : public Mapper<Iterator, InternalNESMapperLayout> {
        InternalNESMapperBase(const Iterator &iterator)
                : Mapper<Iterator, InternalNESMapperLayout>(iterator,
                                                            InternalNESMapperLayout(),
                                                            0x0000)
        {}
    };

    struct InternalNESMapper {
        using Layout = InternalNESMapperLayout;

        template <class Storage>
        using BaseMapper = InternalNESMapperBase<typename Storage::iterator>;
    };

} // namespace nemu