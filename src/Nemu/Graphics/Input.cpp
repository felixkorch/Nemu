#include "Nemu/graphics/Input.h"
#include "Nemu/graphics/Window.h"

namespace nemu {
namespace graphics {
	bool Input::IsKeyPressed(int keycode)
	{
		return Window::GetGlobalInstance()->IsKeyPressed(keycode);
	}

	bool Input::IsJoystickPresent(int number)
	{
		return Window::GetGlobalInstance()->IsJoystickPresent(number);
	}

	bool Input::IsJoystickButtonPressed(int code, int joystick)
	{
		return Window::GetGlobalInstance()->IsJoystickButtonPressed(code, joystick);
	}

	bool Input::IsMouseButtonPressed(int code)
	{
		return Window::GetGlobalInstance()->IsMouseButtonPressed(code);
	}

	double Input::GetMousePositionY()
	{
		return Window::GetGlobalInstance()->GetMousePositionY();
	}

	double Input::GetMousePositionX()
	{
		return Window::GetGlobalInstance()->GetMousePositionX();
	}

	std::pair<double, double> Input::GetMousePosition()
	{
		return Window::GetGlobalInstance()->GetMousePosition();
	}

	std::vector<float> Input::GetJoystickAxis(int joystick)
	{
		return Window::GetGlobalInstance()->GetJoystickAxis(joystick);
	}

}
}