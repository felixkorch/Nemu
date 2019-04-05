#include "Nemu/NESInput.h"
#include <cstddef>
#include <iostream>

namespace nemu
{
	class Joypad {
		NESInput input;
		std::uint8_t shiftReg[2];
		bool strobe;

	public:

		Joypad() :
			shiftReg{},
			strobe(false)
		{}

		void AddInputConfig(const NESInput& _input)
		{
			input = _input;
		}

		// TODO: Some games requires 0x40 / 0x41 to be returned
		std::uint8_t Read(unsigned n)
		{
			// When strobe is high, return the first bit (A)
			if (strobe)
				return input.GetState() & 1;

			// Get the status of a button
			std::uint8_t btn = shiftReg[n] & 1;
			// Read and shift the register to allow the next button to be read
			// A Nintendo brand controller requires a 1 to be shifted in
			shiftReg[n] = Bit7 | (shiftReg[n] >> 1);
			return btn;
		}

		void Write(bool value)
		{
			// 1 -> 0 write is required to read the states
			if (strobe && !value)
				for (int i = 0; i < 2; i++)
					shiftReg[i] = input.GetState();

			strobe = value;
		}
	};
}