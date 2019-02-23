// ---------------------------------------------------------------------* C++ *-
// MaskIterator.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/Mapper/OffsetIterator.h"
#include <cstddef>
#include <iterator>

namespace nemu
{
	template <class Iterator>
	class MaskIterator : public OffsetIterator<MaskIterator<Iterator>> {
		const Iterator itValue;
		const std::size_t maskValue;
		std::size_t offsetValue;

	    public:
		using difference_type = std::ptrdiff_t;
		using value_type = typename Iterator::value_type;
		using pointer = typename Iterator::pointer;
		using reference = typename Iterator::reference;
		using iterator_category = std::random_access_iterator_tag;

		constexpr MaskIterator(const Iterator &it,
				       std::size_t mask,
				       std::size_t offset = 0)
			: itValue(it), maskValue(mask), offsetValue(offset)
		{}

		// Implementing OffsetIterator
		// -------------------------------------------------------------
		constexpr MaskIterator(const MaskIterator &other,
				       std::size_t offset)
			: MaskIterator(other.it(), other.maskValue, offset)
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

		// Implementing Iterator
		// -------------------------------------------------------------
		reference operator*()
		{
			return *std::next(itValue, maskValue & offsetValue);
		}
	};

} // namespace nemu