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
		T &sub()
		{
			return *static_cast<T *>(this);
		}

	    public:
		T operator+(typename std::size_t n)
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

		bool operator!=(const T &other)
		{
			return sub().it() != other.it() ||
			       sub().offset() != other.offset();
		}
	};

} // namespace nemu