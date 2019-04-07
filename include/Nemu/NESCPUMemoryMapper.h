// ---------------------------------------------------------------------* C++ *-
// NESCPUMemoryMapper.h
//
// -----------------------------------------------------------------------------

#pragma once

#include "Nemu/InternalNESMapper.h"
#include <memory>

namespace nemu {

/// Behaves like a mapper of the following format:
///
/// Mapping:
///    InternalMapper:
///           range: (0x0000, 0x401F)
///           size: see InternalNESMapper.
///           mirroring: see InternalNESMapper.
///
///    CartridgeMapper:
///           range: (0x4020, 0xFFFF)
///           size: depends on mapper class.
///           mirroring: depends on mapper class.
///
template <class CartridgeMapper>
class NESCPUMemoryMapper {
    using CPU = CPU<NESCPUMemoryMapper<CartridgeMapper>>;
  public:
    // TODO:
    //  For simplicity everything is shared pointers. There are probably more static solutions.
    std::shared_ptr<InternalNESMapper<CPU>> internalMapper;
    std::shared_ptr<CartridgeMapper> cartridgeMapper;

    std::uint8_t Read(std::size_t address)
    {
        if (address <= 0x401F)
            return internalMapper->Read(address);
        if (address <= 0xFFFF)
            return cartridgeMapper->Read(address);
        return 0;
    }

    void Write(std::size_t address, std::uint8_t value)
    {
        if (address <= 0x401F)
            internalMapper->Write(address, value);
        else if (address <= 0xFFFF)
            cartridgeMapper->Write(address, value);
    }
};

} // namespace nemu