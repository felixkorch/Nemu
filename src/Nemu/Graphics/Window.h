#pragma once
#include "Nemu/Graphics/Event.h"
#include "Nemu/Graphics/Shader.h"
#include "Nemu/Graphics/OpenGL.h"
#include <queue>

namespace nemu::graphics {

	class Window {
		int width, height;
		std::string title;
		bool resizable;
		bool vsync;
		GLFWwindow* window;
		std::queue<EventWrapper> eventQueue;

	public:

		Window(GLFWwindow* window, int width, int height, const std::string& title, bool resizable = false)
			: width(width)
			, height(height)
			, title(title)
			, resizable(resizable)
			, vsync(false)
			, window(window)
			, eventQueue()
		{
			glfwSetWindowUserPointer(window, this);

			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_SAMPLES, 4);

			if (resizable) {
				glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
			}
			else {
				glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
			}

			/*
			glfwSetWindowCloseCallback(window, [](GLFWwindow * window) {
			});

			glfwSetScrollCallback(window, [](GLFWwindow * window, double xOffset, double yOffset) {
			});

			glfwSetCursorPosCallback(window, [](GLFWwindow * window, double xPos, double yPos) {
			});

			glfwSetMouseButtonCallback(window, [](GLFWwindow * window, int button, int action, int mods) {
			});

			glfwSetCharCallback(window, [](GLFWwindow * window, unsigned int keycode) {
			});

			glfwSetKeyCallback(window, [](GLFWwindow * window, int key, int scancode, int action, int mods) {
			});
			*/

			// Callback to set the viewport to match the new size of the window
			glfwSetFramebufferSizeCallback(window, [](GLFWwindow * window, int width, int height) {
				glViewport(0, 0, width, height);
			});

			// Drop Event
			glfwSetDropCallback(window, [](GLFWwindow * window, int count, const char** ptr) {
				Window* win = (Window*)glfwGetWindowUserPointer(window);
				win->eventQueue.push(WrapEvent<DropEvent>(ptr, count));
			});

			// Key Event
			glfwSetKeyCallback(window, [](GLFWwindow * window, int key, int scancode, int action, int mods) {
				Window* win = (Window*)glfwGetWindowUserPointer(window);
				switch (action) {
				case GLFW_PRESS: {
					win->eventQueue.push(WrapEvent<KeyPressEvent>(key));
					break;
				}
				case GLFW_RELEASE: {
					win->eventQueue.push(WrapEvent<KeyReleaseEvent>(key));
					break;
				}
				case GLFW_REPEAT: {
					// TODO
					break;
				}
				}
			});

			glfwSetWindowSizeCallback(window, [](GLFWwindow * window, int width, int height) {
				Window* win = (Window*)glfwGetWindowUserPointer(window);
				win->eventQueue.push(WrapEvent<ResizeEvent>(width, height));
			});
		}

		bool PollEvent(EventWrapper& event)
		{
			if (!eventQueue.empty()) {
				event = eventQueue.front();
				eventQueue.pop();
				return true;
			}
			return false;
		}

		bool IsOpen()
		{
			return !glfwWindowShouldClose(window);
		}

		void Clear()
		{
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		void Update()
		{
			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		void SetVSync(bool enabled)
		{
			if (enabled) {
				glfwSwapInterval(1);
			}
			else {
				glfwSwapInterval(0);
			}
			vsync = enabled;
		}

		bool IsKeyPressed(int keycode)
		{
			auto state = glfwGetKey(window, keycode);
			return state == GLFW_PRESS || state == GLFW_REPEAT;
		}

		bool IsJoystickPresent(int number)
		{
			int present = glfwJoystickPresent(number);
			return present;
		}

		bool IsJoystickButtonPressed(int code, int joystick)
		{
			if (!IsJoystickPresent(joystick))
				return false;

			int count;
			const unsigned char* button = glfwGetJoystickButtons(joystick, &count);
			if (code - 1 > count || code < 0) {
				std::cout << "Joystick Button doesn't exist!\n";
				return false;
			}
			return button[code] == GLFW_PRESS || button[code] == GLFW_REPEAT;
		}

		std::vector<float> GetJoystickAxis(int joystick) // y-axis is inversed?
		{
			if (!IsJoystickPresent(joystick))
				return std::vector<float>();

			int count;
			const float* axis = glfwGetJoystickAxes(joystick, &count);
			return std::vector<float>(axis, axis + count);
		}

		bool IsMouseButtonPressed(int code)
		{
			return glfwGetMouseButton(window, code) == GLFW_PRESS;
		}

		std::pair<double, double> GetMousePosition()
		{
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			return std::make_pair(xpos, ypos);
		}

		double GetMousePositionX()
		{
			return GetMousePosition().first;
		}

		double GetMousePositionY()
		{
			return GetMousePosition().second;
		}

	};

	std::shared_ptr<Window> MakeWindow(int width, int height, const std::string& title)
	{
		if (!glfwInit()) {
			std::cout << "Failed to create window." << std::endl;
			return nullptr;
		}

		auto glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

		if (!glfwWindow) {
			glfwTerminate();
			std::cout << "Failed to create window." << std::endl;
			return nullptr;
		}

		glfwMakeContextCurrent(glfwWindow);

#ifndef NEMU_PLATFORM_WEB

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			std::cout << "Failed to initialize OpenGL" << std::endl;
			return nullptr;
		}
#endif
		std::cout << "Window successfully created" << std::endl;
		return std::make_shared<Window>(glfwWindow, width, height, title);
	}
}