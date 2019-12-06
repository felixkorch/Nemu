// -----------------------------------------------------------------------------------------* C++ *-
// NESInput.h
//
// -------------------------------------------------------------------------------------------------

#pragma once
#include "Nemu/Input.h"

namespace nemu {

	class NESInput {
	public:

		std::uint8_t GetState()
		{
			return
				(Input::IsKeyPressed(SDL_SCANCODE_X) << 0) |
				(Input::IsKeyPressed(SDL_SCANCODE_Z) << 1) |
				(Input::IsKeyPressed(SDL_SCANCODE_LSHIFT) << 2) |
				(Input::IsKeyPressed(SDL_SCANCODE_RETURN) << 3) |
				(Input::IsKeyPressed(SDL_SCANCODE_UP) << 4) |
				(Input::IsKeyPressed(SDL_SCANCODE_DOWN) << 5) |
				(Input::IsKeyPressed(SDL_SCANCODE_LEFT) << 6) |
				(Input::IsKeyPressed(SDL_SCANCODE_RIGHT) << 7);
		}
	};

} // namespace nemu