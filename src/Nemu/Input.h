#pragma once
#include "SDL2/SDL.h"
#undef main
#include "SDL2/SDL_opengl.h"
#include <iostream>

extern SDL_Joystick* joystick0;
namespace nemu::Input {


	inline static bool IsKeyPressed(int keycode)
	{
		const std::uint8_t* state = SDL_GetKeyboardState(NULL);
		return state[keycode];
	}

	inline static bool IsJoystickPresent(int number)
	{
		return number < SDL_NumJoysticks();
	}

	inline static bool IsJoystickHatDown(int number)
	{
		return SDL_JoystickGetHat(joystick0, 0) == number;
	}

	inline static bool IsJoystickButtonDown(int number)
	{
		return SDL_JoystickGetButton(joystick0, number);
	}

	inline static int GetJoystickCount()
	{
		return SDL_NumJoysticks();
	}
}