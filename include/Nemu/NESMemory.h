// ---------------------------------------------------------------------* C++ *-
// NESMemory.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/Mapper/InternalNESMapper.h"
#include <cstddef>
#include <vector>
#include <type_traits>
#include <iostream>
#include <ios>
#include <iomanip>

namespace nemu
{
    template <class Storage, class Mapper>
    class NESMemoryBase {
        using BaseMapper = typename Mapper::template BaseMapper<Storage>;
        using Layout = typename Mapper::Layout;

        Storage memory;

    public:
        NESMemoryBase(const Layout &layout)
                : memory(layout.AllocSize())
        {}

        // TODO: Should not use class based mapper and layout.
        typename Storage::reference operator[](std::size_t address)
        {
            return *std::next(BaseMapper(memory.begin()), address);
        }

        constexpr bool ContainsAddress(std::size_t address) const
        {
            return Layout().ContainsAddress(address);
        }
    };

    template <class Storage, class Mapper>
    class NESMemory {
        class Iterator {
            NESMemory &memory;
            std::size_t address;

        public:
            constexpr Iterator(NESMemory &memory, std::size_t address = 0)
                    : memory(memory)
                    , address(address)
            {}

            // Implementing RandomAccessIterator
            // -----------------------------------------------------------------
            using difference_type = std::ptrdiff_t;
            using value_type = typename Storage::value_type;
            using pointer = value_type *;
            using reference = value_type &;
            using iterator_category = std::random_access_iterator_tag;

            value_type &operator*()
            {
                return memory[address];
            }

            Iterator &operator++()
            {
                ++address;
                return *this;
            }

            Iterator &operator--()
            {
                --address;
                return *this;
            }

            constexpr Iterator operator+(difference_type offset) const
            {
                return Iterator(memory, address + offset);
            }

            constexpr Iterator operator-(difference_type offset) const
            {
                return Iterator(memory, address - offset);
            }

            constexpr difference_type operator-(const Iterator &other) const
            {
                return other.address - address;
            }

            Iterator &operator+=(difference_type offset)
            {
                address += offset;
                return *this;
            }

            Iterator &operator-=(difference_type offset)
            {
                address -= offset;
                return *this;
            }

            constexpr bool operator!=(const Iterator &other) const
            {
                return &memory != &other.memory || address != other.address;
            }

            constexpr bool operator==(const Iterator &other) const
            {
                return !((*this) != other);
            }
        }; // class Iterator
    public:
        using value_type = typename Storage::value_type;
        using reference = value_type &;
        using const_reference = const value_type &;
        using iterator = Iterator;
        using const_iterator = const iterator;
        using difference_type = std::ptrdiff_t;
        using size_type = std::size_t;

    private:
        value_type nullRef;

        NESMemoryBase<Storage, InternalNESMapper> internalMemory;
        NESMemoryBase<Storage, Mapper> externalMemory;

        value_type &GetRef(std::size_t address)
        {
            if (internalMemory.ContainsAddress(address))
                return internalMemory[address];
            if (externalMemory.ContainsAddress(address))
                return externalMemory[address];
            return nullRef;
        }

    public:
        NESMemory()
                : internalMemory(InternalNESMapperLayout())
                , externalMemory(typename Mapper::Layout())
        {}

        std::uint16_t Get16At(std::size_t address)
        {
            return (*this)[address] | ((*this)[address + 1] << 8);
        }

        void Dump()
        {
            int i = 0;

            for (const auto &element : *this) {
                std::cout << std::hex << std::setfill('0') << "0x" << std::setw(4) << i << ": 0x"
                          << std::setw(4) << element << '\n';
                ++i;
            }
        }

        // Implementing Container
        //
        // TODO:
        //	Complete implementation.
        // ---------------------------------------------------------------------

        value_type &operator[](std::size_t address)
        {
            return GetRef(address);
        }

        // TODO:
        //   Not very important but it would be convinient to have a const
        //   operator[].
        //
        // const value_type &operator[](std::size_t address) const
        // {
        //     return GetRef(address);
        // }

        constexpr size_type size() const
        {
            return 0x00010000;
        }

        iterator begin()
        {
            return Iterator(*this, 0);
        }

        iterator end()
        {
            return Iterator(*this, size());
        }

        constexpr const_iterator cbegin() const
        {
            return begin();
        }

        constexpr const_iterator cend() const
        {
            return end();
        }
    };

} // namespace nemu