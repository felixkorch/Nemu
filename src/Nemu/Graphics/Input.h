#pragma once
#include "Nemu/graphics/Window.h"
#include <memory>

namespace nemu {
namespace graphics {

	class Input {
	public:
		static bool IsKeyPressed(int keycode);
		static bool IsJoystickPresent(int number);
		static bool IsJoystickButtonPressed(int code, int joystick);
		static bool IsMouseButtonPressed(int code);
		static double GetMousePositionY();
		static double GetMousePositionX();
		static std::pair<double, double> GetMousePosition();
		static std::vector<float> GetJoystickAxis(int joystick);
	};

}
}