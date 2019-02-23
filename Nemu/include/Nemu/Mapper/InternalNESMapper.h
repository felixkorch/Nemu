// ---------------------------------------------------------------------* C++ *-
// InternalNESMapper.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/Mapper/MaskIterator.h"
#include "Nemu/Mapper/OffsetIterator.h"
#include <cstddef>

namespace nemu
{
    template <class Iterator>
    class InternalNESMapperBase
            : OffsetIterator<InternalNESMapperBase<Iterator>> {
        static MaskIterator<Iterator> InternalRAM(const Iterator &it,
                                                  std::size_t offset)
        {
            return MaskIterator<Iterator>(it, 0x07FF, offset);
        }

        static MaskIterator<Iterator> PPURegisters(const Iterator &it,
                                                   std::size_t offset)
        {
            return MaskIterator<Iterator>(
                    std::next(it, 0x0800), 0x0007, offset);
        }

        static Iterator APUIORegisters(const Iterator &it, std::size_t offset)
        {
            return std::next(std::next(it, 0x0808), offset - 0x4000);
        }

        const Iterator itValue;
        std::size_t offsetValue;

    public:
        static constexpr bool ContainsAddress(std::size_t address)
        {
            return address < 0x4020;
        }

        // Implements Iterator
        // -------------------------------------------------------------
        using difference_type = std::ptrdiff_t;
        using value_type = typename Iterator::value_type;
        using pointer = typename Iterator::pointer;
        using reference = typename Iterator::reference;
        using iterator_category = std::random_access_iterator_tag;

        reference operator*()
        {
            if (offsetValue < 0x2000)
                return *InternalRAM(itValue, offsetValue);
            if (offsetValue < 0x4000)
                return *PPURegisters(itValue, offsetValue);
            return *APUIORegisters(itValue, offsetValue);
        }

        // Implements OffsetIterator
        // -------------------------------------------------------------
        InternalNESMapperBase(const Iterator &it, difference_type offset)
                : itValue(it), offsetValue(offset)
        {}

        constexpr Iterator it() const
        {
            return itValue;
        }

        constexpr std::size_t offset() const
        {
            return offsetValue;
        }

        std::size_t &offset()
        {
            return offsetValue;
        }
    };

    /// Mapping of the non cartrigde memory:
    /// Source: 'https://wiki.nesdev.com/w/index.php/CPU_memory_map'
    ///
    /// Note that 'size' refers to the non mirrored partion of the
    /// memory.
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
    struct InternalNESMapper {
        template <class Storage>
        using BaseMapper = InternalNESMapperBase<typename Storage::iterator>;

        /// Amount of cells needed to allocate for the internal NES
        /// memory.
        static constexpr std::size_t AllocSize()
        {
            return 0x0828;
        }
    };

} // namespace nemu