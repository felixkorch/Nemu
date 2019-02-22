// ---------------------------------------------------------------------* C++ *-
// InternalNESMapper.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/Mapper/MaskIterator.h"
#include <cstddef>

namespace nemu
{
	static constexpr std::size_t InternalNESMemorySize()
	{
		return 0x0828;
	}

	template <class Iterator>
	class InternalNESMapper {
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
		/// Cartrigde space
		///     range: (0x4020, 0xFFFF)
		///
		const Iterator it;
		std::size_t offset;

		static MaskIterator<Iterator> InternalRAM(const Iterator &it,
							  std::size_t offset)
		{
			return MaskIterator<Iterator>(it, 0x07FF, offset);
		}

		static MaskIterator<Iterator> PPURegisters(const Iterator &it,
							   std::size_t offset)
		{
			return MaskIterator<Iterator>(
				std::next(it, 0x0800), 0x0007, offset - 0x2000);
		}

		static Iterator APUIORegisters(const Iterator &it,
					       std::size_t offset)
		{
			return std::next(std::next(it, 0x0808),
					 offset - 0x4000);
		}

	    public:
		using difference_type = std::size_t;
		using value_type = typename Iterator::value_type;
		using pointer = typename Iterator::pointer;
		using reference = typename Iterator::reference;
		using iterator_category = std::random_access_iterator_tag;

		InternalNESMapper(const Iterator &it, difference_type offset)
			: it(it), offset(offset)
		{}

		static constexpr bool ContainsAddress(std::size_t address)
		{
			return address < 0x4020;
		}

		reference operator*()
		{
			if (offset < 0x2000)
				return *InternalRAM(it, offset);
			if (offset < 0x4000)
				return *PPURegisters(it, offset);
			return *APUIORegisters(it, offset);
		}

		InternalNESMapper<Iterator> &operator++()
		{
			++offset;
			return *this;
		}

		InternalNESMapper<Iterator> &operator+=(difference_type n)
		{
			offset += n;
			return *this;
		}

		InternalNESMapper<Iterator> operator+(difference_type n)
		{
			return NESMapper(it, offset + n);
		}

		bool operator!=(const InternalNESMapper<Iterator> &other)
		{
			return it != other.it || offset != other.offset;
		}
	};

} // namespace nemu