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
	template <class Iterator>
	class NROM256MapperBase : OffsetIterator<NROM256MapperBase<Iterator>> {
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
			return address > 0x7FFF && address <= 0xFFFF;
		}

		static constexpr std::size_t AllocSize()
		{
			return 0x7FFF;
		}

		// Implements Iterator
		// -------------------------------------------------------------
		reference operator*()
		{
			return *std::next(itValue, offsetValue - 0x8000);
		}

		// Implements OffsetIterator
		// -------------------------------------------------------------
		NROM256MapperBase(const Iterator &it,
				  difference_type offset = 0x8000)
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

	/// Provides mapping for the NROM-256 cartridge layout.
	///
	/// Mapping:
	///     chunk:
	///             range: (0x8000, 0xFFFF)
	///             size: 0x7FFF (32kB)
	///
	/// TODO:
	///     Mirroring of NROM-256 is hardware specific so the
	///     mapper should be configurable. But for now one chunk of
	///     32kB memory will do.
	///
	struct NROM256Mapper {
		template <class Storage>
		using BaseMapper =
			NROM256MapperBase<typename Storage::iterator>;

		/// Amount of cells needed to allocate for the NROM memory
		/// (32kB).
		static constexpr std::size_t AllocSize()
		{
			return 0x7FFF;
		}
	};

} // namespace nemu