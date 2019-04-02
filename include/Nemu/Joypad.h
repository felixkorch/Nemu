#include "Nemu/NESInput.h"
#include <cstddef>
#include <iostream>

namespace nemu
{
	class Joypad {
		NESInput input;
		std::uint8_t states[2];
		bool strobe;

	public:

		Joypad() :
			states{},
			strobe(false)
		{}

		void AddInputConfig(const NESInput& _input)
		{
			input = _input;
		}

		std::uint8_t Read(unsigned n)
		{
			// When strobe is high, it keeps reading A:
			if (strobe)
				return 0x40 | (input.GetState() & 1);

			// Get the status of a button and shift the register:
			std::uint8_t j = 0x40 | (states[n] & 1);
			states[n] = 0x80 | (states[n] >> 1);
			return j;
		}

		void Write(bool v)
		{
			// Read the joypad data on strobe's transition 1 -> 0.
			if (strobe && !v)
				for (int i = 0; i < 2; i++)
					states[i] = input.GetState();

			strobe = v;
		}
	};
}