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
		static std::shared_ptr<Window> globalInstance;

	public:

		Window(int width, int height, const std::string& title, bool resizable)
			: width(width)
			, height(height)
			, title(title)
			, resizable(resizable)
			, vsync(false)
			, window(nullptr)
			, eventQueue()
		{}
		
		static std::shared_ptr<Window> Create(int width, int height, const std::string& title, bool resizable = false)
		{
			assert(globalInstance == nullptr);
			globalInstance = std::make_shared<Window>(width, height, title, resizable);
			if (!globalInstance->TryInit())
				return nullptr;
			return globalInstance;
		}

		bool PollEvent(EventWrapper & event)
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

		bool TryInit()
		{
			if (!glfwInit()) {
				std::cout << "Failed to initialize window." << std::endl;
				return false;
			}

			window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

			if (!window) {
				glfwTerminate();
				std::cout << "Failed to initialize window." << std::endl;
				return false;
			}

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

			glfwMakeContextCurrent(window);
			glfwSetWindowUserPointer(window, this);

#ifndef NEMU_PLATFORM_WEB

			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
				std::cout << "Failed to initialize OpenGL" << std::endl;
				return false;
			}
#endif

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
				auto e = std::make_shared<DropEvent>(ptr, count);
				win->eventQueue.push(EventWrapper(e));
				});

			// Key Event
			glfwSetKeyCallback(window, [](GLFWwindow * window, int key, int scancode, int action, int mods) {
				Window* win = (Window*)glfwGetWindowUserPointer(window);
				switch (action) {
				case GLFW_PRESS: {
					auto e = std::make_shared<KeyPressEvent>(key);
					win->eventQueue.push(EventWrapper(e));
					break;
				}
				case GLFW_RELEASE: {
					auto e = std::make_shared<KeyReleaseEvent>(key);
					win->eventQueue.push(EventWrapper(e));
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
				auto e = std::make_shared<ResizeEvent>(width, height);
				win->eventQueue.push(EventWrapper(e));
				});


			return true;
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

		template <class T>
		void Draw(T& type, Shader& shader)
		{
			type.Draw(width, height, shader);
		}

		static std::shared_ptr<Window> GetGlobalInstance() { return globalInstance; }

	};

	inline std::shared_ptr<Window> Window::globalInstance = nullptr;
}