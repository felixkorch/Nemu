#pragma once
#include "Nemu/Graphics/Event.h"
#include "Nemu/Graphics/Shader.h"
#include "GLFW/glfw3.h"
#include <queue>

namespace nemu {
namespace graphics {

	class Window {
		int width, height;
		std::string title;
		bool resizable;
		bool vsync;
		GLFWwindow* window;
		std::queue<EventWrapper> eventQueue;
		static std::shared_ptr<Window> globalInstance;

	public:
		Window(int width, int height, const std::string& title, bool resizable);
		bool PollEvent(EventWrapper& event);
		bool IsOpen();
		void Clear();
		void Update();
		void SetVSync(bool enabled);
		bool TryInit();

		bool IsKeyPressed(int keycode);
		bool IsJoystickPresent(int number);
		bool IsJoystickButtonPressed(int code, int joystick);
		bool IsMouseButtonPressed(int code);
		double GetMousePositionX();
		double GetMousePositionY();
		std::vector<float> GetJoystickAxis(int joystick);
		std::pair<double, double> GetMousePosition();

		template <class T>
		void Draw(T & type, Shader& shader)
		{
			type.Draw(width, height, shader);
		}

		static std::shared_ptr<Window> Create(int width, int height, const std::string& title, bool resizable = false);
		static std::shared_ptr<Window> GetGlobalInstance() { return globalInstance; }

	};

}
}