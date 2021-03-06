// -----------------------------------------------------------------------------------------* C++ *-
// PPUMapper.h
//
// -------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <memory>

namespace nemu::mapper {

template <class CartridgeMapper>
class PPUMapper {
   public:
    std::shared_ptr<CartridgeMapper> cartridgeMapper;

	PPUMapper()
		: cartridgeMapper(nullptr)
	{}

    std::uint8_t Read(std::size_t address)
    {
        if (address <= 0x1FFF)
            return cartridgeMapper->ReadCHR(address);
        // TODO:
        //   Move PPUs internal memory to this class?
        return 0;
    }

    void Write(std::size_t address, std::uint8_t value)
    {
        if (address <= 0x1FFF)
            cartridgeMapper->WriteCHR(address, value);
        // TODO:
        //   Move PPUs internal memory to this class?
    }

    void OnScanline()
    {
        cartridgeMapper->OnScanline();
    }
};

} // namespace mapper