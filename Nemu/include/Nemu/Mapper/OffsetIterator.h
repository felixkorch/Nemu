// ---------------------------------------------------------------------* C++ *-
// OffsetIterator.h
//
// -----------------------------------------------------------------------------

#pragma once
#include <cstddef>
#include <iterator>

namespace nemu
{
    template <class T>
    class OffsetIterator {
        constexpr T sub() const
        {
            return *static_cast<const T *>(this);
        }

        T &sub()
        {
            return *static_cast<T *>(this);
        }

    public:
        constexpr T operator+(typename std::size_t n) const
        {
            return T(sub(), sub().offset() + n);
        }

        T &operator+=(typename std::size_t n)
        {
            sub().offset() += n;
            return sub();
        }

        T &operator++()
        {
            ++(sub().offset());
            return sub();
        }

        constexpr bool operator!=(const T &other) const
        {
            return sub().it() != other.it() || sub().offset() != other.offset();
        }
    };

} // namespace nemu