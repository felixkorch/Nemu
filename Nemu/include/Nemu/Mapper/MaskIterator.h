// ---------------------------------------------------------------------* C++ *-
// MaskIterator.h
//
// -----------------------------------------------------------------------------

#pragma once

#include <cstddef>
#include <iterator>

namespace nemu
{
template <class Iterator> class MaskIterator {
	const Iterator it;
	const std::size_t mask;
	std::size_t offset;

    public:
	using difference_type = std::size_t;
	using value_type = typename Iterator::value_type;
	using pointer = typename Iterator::pointer;
	using reference = typename Iterator::reference;
	using iterator_category = std::random_access_iterator_tag;

	constexpr MaskIterator(const Iterator &it,
			       std::size_t mask,
			       std::size_t offset = 0)
		: it(it), mask(mask), offset(offset)
	{
	}

	MaskIterator operator+(difference_type n)
	{
		return MaskIterator(it, mask, offset + n);
	}

	MaskIterator &operator+=(difference_type n)
	{
		offset += n;
		return *this;
	}

	MaskIterator &operator++()
	{
		++offset;
		return *this;
	}

	value_type &operator*()
	{
		return *std::next(it, offset & mask);
	}

	bool operator!=(const MaskIterator<Iterator> &other)
	{
		return it != other.it || offset != other.offset;
	}
};

} // namespace nemu