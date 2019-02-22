// ---------------------------------------------------------------------* C++ *-
// NESMemory.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/Mapper/InternalNESMapper.h"
#include <cstddef>
#include <vector>

namespace nemu
{
	template <class Storage, class Mapper>
	class NESMemoryBase {
		Storage memory;

	    public:
		NESMemoryBase(Storage &&memory) : memory(std::move(memory))
		{}

		typename Storage::reference operator[](std::size_t address)
		{
			return *(Mapper(memory.begin(), address));
		}

		constexpr bool ContainsAddress(std::size_t address) const
		{
			return Mapper::ContainsAddress(address);
		}
	};

	template <class Storage, class Mapper>
	class NESMemory {
		using Cell = typename Storage::value_type;
		using InternalStorage = std::vector<Cell>;

		Cell nullRef;

		NESMemoryBase<Storage,
			      InternalNESMapper<typename Storage::iterator>>
			internalMemory;
		NESMemoryBase<Storage, Mapper> externalMemory;

	    public:
		NESMemory(Storage &&externalMemory)
			: internalMemory(Storage(InternalNESMemorySize())),
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
	};

	template <class Storage, class Mapper>
	NESMemory<Storage, Mapper> MakeNESMemory(Storage &&storage)
	{
		return NESMemory<Storage, Mapper>(std::move(storage));
	}

} // namespace nemu