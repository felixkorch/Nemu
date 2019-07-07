#include "Nemu/Graphics/Window.h"
#include "Nemu/Graphics/Input.h"
#include "Nemu/Graphics/Event.h"
#include "Nemu/Graphics/Shader.h"
#include "Nemu/Graphics/Texture.h"
#include "Nemu/Core/NESInstance.h"
#include "Nemu/Core/ROMLayout.h"
#include "Nemu/Core/Utilities.h"

#include <ctime>
#include <chrono>
#include <thread>

using namespace nemu;
using namespace graphics;

const char* simpleShader = {
	#include "Nemu/Graphics/Shaders/texture.shader"
};

int main()
{
	auto window = Window::Create(512, 480, "Nemu");
	if (window == nullptr)
		return 1;

	// Graphic elements
	Texture2D texture(256, 240);
	texture.Resize(512, 480);
	Shader textureShader;
	textureShader.LoadFromString(simpleShader);

	// Game state
	std::unique_ptr<NESInstance> nesInstance;
	std::unique_ptr<NESInstance> state;
	bool running = false;
	auto delay = std::chrono::steady_clock::now();
	NESInput input;
	input.SetKeyboardConfig(NESKeyMapper::DefaultMap());

	auto PPUCallback = [&texture](std::uint8_t* pixels) { texture.SetData(pixels); };

	while (window->IsOpen()) {
		window->Clear();

		// Poll Events
		EventWrapper event;
		while (window->PollEvent(event)) {

			if (event.GetEventType() == EventType::DropEvent) {
				DropEvent& e = event;
				if (e.Count() > 1)
					return 1;

				if (!StringEndsWith(e.GetPaths()[0], ".nes"))
					return 1;

				running = true;

				nesInstance = MakeNESInstance((NESInstance::Descriptor{
					ROMLayout(e.GetPaths()[0]),
					input,
					PPUCallback,
					1
				}));

				if (nesInstance == nullptr) {
					std::cout << "Failed to load ROM\n";
					return 1;
				}
				nesInstance->Power();
			}
			else if (event.GetEventType() == EventType::KeyPressEvent) {
				KeyPressEvent& e = event;
				switch (e.GetKey()) {
				case GLFW_KEY_F1: {
					*state = *nesInstance;
					std::cout << "State saved!\n";
					break;
				}
				case GLFW_KEY_F2: {
					*nesInstance = *state;
					std::cout << "State loaded!\n";
					break;
				}
				}
			}

		}

		// Update states
		if (running)
			nesInstance->RunFrame();


		// Draw
		window->Draw(texture, textureShader);
		window->Update();

		// Delay to force 60fps
		std::this_thread::sleep_until(delay);
		delay += std::chrono::nanoseconds(1000000000) / 60;
	}
}