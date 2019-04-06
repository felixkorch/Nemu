#pragma once
#include <cstddef>
#include <iostream>

namespace nemu {

	class NESMapper {
	public:
		virtual void Write(std::size_t address, std::uint8_t value) = 0;
		virtual std::uint8_t Read(std::size_t address) = 0;
	};

}