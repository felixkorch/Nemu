#include "Nemu/Graphics/Window.h"
#include "Nemu/Graphics/Input.h"
#include "Nemu/Graphics/Event.h"
#include "Nemu/Graphics/Shader.h"
#include "Nemu/Graphics/VertexBuffer.h"
#include "Nemu/Graphics/Texture.h"
#include "Nemu/Core/NESInstance.h"
#include "Nemu/Core/ROMLayout.h"
#include "Nemu/Core/Utilities.h"

#include <ctime>
#include <chrono>
#include <thread>

using namespace nemu;
using namespace graphics;

#ifdef NEMU_PLATFORM_WEB
const char* minimalShader = {
	#include "Nemu/Graphics/Shaders/texture.gles3.shader"
};
#else
const char* minimalShader = {
	#include "Nemu/Graphics/Shaders/texture.shader"
};
#endif

// Hack for emscripten
static void CallMain(void* fp)
{
	std::function<void()>* fn = (std::function<void()>*) fp;
	(*fn)();
}

int main()
{
	auto window = MakeWindow(512, 480, "Nemu");
	if (window == nullptr)
		return 1;

	Input::SetSourceWindow(window);

	// Graphic elements
	Texture2D texture(256, 240);
	texture.Resize(512, 480);
	Shader textureShader = Shader::CreateFromString(minimalShader);

	// Game objects
	std::unique_ptr<NESInstance> nesInstance, state;
	bool running = false;
	auto delay = std::chrono::steady_clock::now();
	NESInput input;
	input.SetKeyboardConfig(NESKeyMapper::DefaultMap());

#ifdef NEMU_PLATFORM_WEB
	std::function<void()> mainLoop = [&]() {
#else
	while (window->IsOpen()) {
#endif

		window->Clear();

		// Poll Events
		EventWrapper event;
		while (window->PollEvent(event)) {


			if (event->GetEventType() == EventType::DropEvent) {
				DropEvent& e = event;

				if (e.Count() > 1)
					return 1;

				if (!util::StringEndsWith(e.GetPaths()[0], ".nes"))
					return 1;

				running = true;

				nesInstance = MakeNESInstance(NESInstance::Descriptor {
					ROMLayout(e.GetPaths()[0]),
					input,
					-1
				});

				if (nesInstance == nullptr) {
					std::cout << "Failed to load ROM\n";
					return 1;
				}
				nesInstance->Power();
			}
			else if (event->GetEventType() == EventType::KeyPressEvent) {
				KeyPressEvent& e = event;
				switch (e.GetKey()) {
                case GLFW_KEY_F1: {
					state = nesInstance->Clone();
                    std::cout << "State saved!\n";
                    break;
                }
                case GLFW_KEY_F2: {
					nesInstance = state->Clone();
                    std::cout << "State loaded!\n";
                    break;
                }
				}
			}

		}

		// Update states
		if (running) {
			nesInstance->RunFrame();
			texture.SetData(nesInstance->GetPixels());
		}

		// Draw
		window->Draw(texture, textureShader);
		window->Update();

		// Delay to force 60fps
		std::this_thread::sleep_until(delay);
		delay += std::chrono::nanoseconds(1000000000) / 60;

#ifdef NEMU_PLATFORM_WEB
	};
	emscripten_set_main_loop_arg(CallMain, &mainLoop, 0, 1);
#else
	}
#endif
}
