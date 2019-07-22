#pragma once
#include "Nemu/Graphics/Window.h"
#include <memory>

namespace nemu::graphics {

	class Window;

	class Input {
		static std::shared_ptr<Window> sourceWindow;
	public:

		static void SetSourceWindow(std::shared_ptr<Window> win) { sourceWindow = win; }

		static bool IsKeyPressed(int keycode)
		{
			return sourceWindow->IsKeyPressed(keycode);
		}

		static bool IsJoystickPresent(int number)
		{
			return sourceWindow->IsJoystickPresent(number);
		}

		static bool IsJoystickButtonPressed(int code, int joystick)
		{
			return sourceWindow->IsJoystickButtonPressed(code, joystick);
		}

		static bool IsMouseButtonPressed(int code)
		{
			return sourceWindow->IsMouseButtonPressed(code);
		}

		static double GetMousePositionY()
		{
			return sourceWindow->GetMousePositionY();
		}

		static double GetMousePositionX()
		{
			return sourceWindow->GetMousePositionX();
		}

		static std::pair<double, double> GetMousePosition()
		{
			return sourceWindow->GetMousePosition();
		}

		static std::vector<float> GetJoystickAxis(int joystick)
		{
			return sourceWindow->GetJoystickAxis(joystick);
		}
	};

	inline std::shared_ptr<Window> Input::sourceWindow = nullptr;
}