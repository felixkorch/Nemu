// ---------------------------------------------------------------------* C++ *-
// NESMemory.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/Mapper/InternalNESMapper.h"
#include <cstddef>
#include <vector>
#include <type_traits>

namespace nemu
{
    template <class Storage, class MapperBase>
    class NESMemoryBase {
        Storage memory;

    public:
        NESMemoryBase(Storage &&memory) : memory(std::move(memory))
        {}

        typename Storage::reference operator[](std::size_t address)
        {
            return *(MapperBase(memory.begin(), address));
        }

        constexpr bool ContainsAddress(std::size_t address) const
        {
            return MapperBase::ContainsAddress(address);
        }
    };

    template <class Storage, class Mapper>
    class NESMemory {
        using Cell = typename Storage::value_type;

        Cell nullRef;

        NESMemoryBase<Storage, InternalNESMapper::BaseMapper<Storage>>
                internalMemory;
        NESMemoryBase<Storage, Mapper> externalMemory;

        class Iterator {
            NESMemory &memory;
            std::size_t address;

        public:
            Iterator(NESMemory &memory, std::size_t address = 0)
                    : memory(memory), address(address)
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

            Iterator & operator++()
            {
                ++address;
                return *this;
            }

			Iterator & operator--()
            {
                --address;
                return *this;
            }

			Iterator operator+(difference_type offset) const 
			{
				return Iterator(memory, address + offset);
			}

            Iterator operator-(difference_type offset) const
			{
				return Iterator(memory, address - offset);
			}

            difference_type operator-(const Iterator& other) const
            {
                return other.address - address;
            }

			Iterator& operator+=(difference_type offset) 
			{
				address += offset;
				return *this;
			}

            Iterator& operator-=(difference_type offset) 
			{
				address -= offset;
				return *this;
			}

            bool operator!=(const Iterator &other)
            {
                return &memory != &other.memory || address != other.address;
            }

            bool operator==(const Iterator &other)
            {
                return !((*this) != other);
            }
        };

    public:
        constexpr NESMemory(Storage &&internalMemory, Storage &&externalMemory)
                : internalMemory(std::move(internalMemory)),
                  externalMemory(std::move(externalMemory))
        {}

        Cell &operator[](std::size_t address)
        {
            if (internalMemory.ContainsAddress(address))
                return internalMemory[address];
            if (externalMemory.ContainsAddress(address))
                return externalMemory[address];
            return nullRef;
        }

        std::uint16_t Get16At(std::size_t address)
        {
            return (*this)[address] | ((*this)[address + 1] << 8);
        }

        // Implementing Container
        //
        // TODO:
        //	Complete implementation.
        // ---------------------------------------------------------------------
        using value_type = typename Storage::value_type;
        using reference = value_type &;
        using const_reference = const value_type &;
        using iterator = Iterator;
        using const_iterator = const iterator;
        using difference_type = std::size_t;
        using size_type = std::size_t;

		constexpr size_type size() const
		{
			return InternalNESMapper::AllocSize() + Mapper::AllocSize();
		}

        iterator begin()
        {
            return Iterator(*this, 0);
        }

		iterator end()
		{
			return Iterator(*this, size());
		}
    };

    template <class Storage, class MapperBase>
    NESMemory<Storage, MapperBase> MakeNESMemory(Storage &&internalMemory,
                                                 Storage &&externalMemory)
    {
        return NESMemory<Storage, MapperBase>(std::move(internalMemory),
                                              std::move(externalMemory));
    }

    /// Creates a container for a NESMemory instance. Uses
    /// InternalNESMapping for the internal memory and Mapper as mapping for
    /// the external memory.
    template <class Storage,
              class Mapper,
              class MapperBase = typename Mapper::template BaseMapper<Storage>,
              typename = typename std::enable_if<std::is_same<
                      Storage,
                      std::vector<typename Storage::value_type>>::value>::type>
    NESMemory<Storage, MapperBase> MakeNESMemory()
    {
        return NESMemory<Storage, MapperBase>(
                Storage(InternalNESMapper::AllocSize()),
                Storage(Mapper::AllocSize()));
    }

} // namespace nemu