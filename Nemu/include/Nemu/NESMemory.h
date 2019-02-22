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

	    public:
		constexpr NESMemory(Storage &&internalMemory,
				    Storage &&externalMemory)
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
	};

	template <class Storage, class MapperBase>
	NESMemory<Storage, MapperBase> MakeNESMemory(Storage &&internalMemory,
						     Storage &&externalMemory)
	{
		return NESMemory<Storage, MapperBase>(
			std::move(internalMemory), std::move(externalMemory));
	}

	/// Creates a container for a NESMemory instance. Uses
	/// InternalNESMapping for the internal memory and Mapper as mapping for
	/// the external memory.
	template <
		class Storage,
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