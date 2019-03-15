// ---------------------------------------------------------------------* C++ *-
// Mapper.h
//
// -----------------------------------------------------------------------------

#pragma once

#include <cstddef>
#include <iterator>

namespace nemu
{
    template <class Iterator, class Layout>
    class Mapper {
        using Self = Mapper<Iterator, Layout>;

        const Iterator iterator;
        const Layout layout;
        std::size_t offset;

    public:
        Mapper(const Iterator &iterator, const Layout &layout, std::size_t offset = 0)
                : iterator(iterator)
                , layout(layout)
                , offset(offset)
        {}

        constexpr std::size_t AllocSize() const
        {
            return layout.AllocSize();
        }

        constexpr bool ContainsAddress(std::size_t address) const
        {
            return layout.ContainsAddress(address);
        }

        // Implements Random Access Iterator
        // -------------------------------------------------------------
        using difference_type = std::ptrdiff_t;
        using value_type = typename Iterator::value_type;
        using pointer = typename Iterator::pointer;
        using reference = typename Iterator::reference;
        using iterator_category = std::random_access_iterator_tag;

        reference operator*()
        {
            return *(layout.Next(iterator, offset));
        }

        constexpr Self operator+(typename std::size_t n) const
        {
            return Self(iterator, layout, offset + n);
        }

        constexpr Self operator-(typename std::size_t n) const
        {
            return (*this + (-n));
        }

        Self &operator+=(typename std::size_t n)
        {
            offset += n;
            return *this;
        }

        Self &operator-=(typename std::size_t n)
        {
            return (*this += -n);
        }

        Self &operator++()
        {
            return (*this += 1);
        }

        Self &operator--()
        {
            return (*this -= 1);
        }

        constexpr bool operator!=(const Self &other) const
        {
            return iterator != other.iterator || offset != other.offset;
        }
    };

} // namespace nemu