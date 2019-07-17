#pragma once
#include "Nemu/graphics/Window.h"
#include <memory>

namespace nemu::graphics {

	class Input {
	public:

		static bool IsKeyPressed(int keycode)
		{
			return Window::GetGlobalInstance()->IsKeyPressed(keycode);
		}

		static bool IsJoystickPresent(int number)
		{
			return Window::GetGlobalInstance()->IsJoystickPresent(number);
		}

		static bool IsJoystickButtonPressed(int code, int joystick)
		{
			return Window::GetGlobalInstance()->IsJoystickButtonPressed(code, joystick);
		}

		static bool IsMouseButtonPressed(int code)
		{
			return Window::GetGlobalInstance()->IsMouseButtonPressed(code);
		}

		static double GetMousePositionY()
		{
			return Window::GetGlobalInstance()->GetMousePositionY();
		}

		static double GetMousePositionX()
		{
			return Window::GetGlobalInstance()->GetMousePositionX();
		}

		static std::pair<double, double> GetMousePosition()
		{
			return Window::GetGlobalInstance()->GetMousePosition();
		}

		static std::vector<float> GetJoystickAxis(int joystick)
		{
			return Window::GetGlobalInstance()->GetJoystickAxis(joystick);
		}
	};
}