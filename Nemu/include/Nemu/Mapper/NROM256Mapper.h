// ---------------------------------------------------------------------* C++ *-
// NROM256Mapper.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/Mapper/OffsetIterator.h"
#include <cstddef>
#include <iterator>

namespace nemu
{
	static constexpr std::size_t NROM256MemorySize()
	{
		return 0x2000;
	}

	template <class Iterator>
	class NROM256Mapper : OffsetIterator<NROM256Mapper<Iterator>> {
		/// TODO:
		///     Mirroring of NROM-256 is hardware specific so the
		///     mapper should be configurable. But for now one chunk of
		///     32kB memory will do.
		///
		/// Mapping:
		///     chunk:
		///             range: (0x8000, 0xFFFF)
		///             size: 0x2000
		///

		const Iterator itValue;
		std::size_t offsetValue;

	    public:
		using difference_type = std::size_t;
		using value_type = typename Iterator::value_type;
		using pointer = typename Iterator::pointer;
		using reference = typename Iterator::reference;
		using iterator_category = std::random_access_iterator_tag;

		static constexpr bool ContainsAddress(std::size_t address)
		{
			return address > 0x7FFF && address < 0xFFFF;
		}

		NROM256Mapper(const Iterator &it,
			      difference_type offset = 0x8000)
			: itValue(it), offsetValue(offset)
		{}

		reference operator*()
		{
			return *std::next(itValue, offsetValue - 0x8000);
		}
	};

} // namespace nemu