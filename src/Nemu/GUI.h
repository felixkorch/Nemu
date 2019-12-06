#pragma once
#include "Nemu/Core/NESInstance.h"
#include "Nemu/Core/ROMLayout.h"
#include "Nemu/Utilities.h"

#include "SDL2/SDL.h"
#undef main
#include "SDL2/SDL_opengl.h"
#include "SDL2/SDL_ttf.h"
#ifdef NEMU_PLATFORM_WEB
#include "emscripten/emscripten.h"
#include "emscripten/html5.h"
#endif

#include <ctime>
#include <chrono>
#include <thread>
#include <memory>
#include <iostream>
#include <sstream>

inline SDL_Joystick* joystick0 = NULL; // Only one joystick supported (device 0)

namespace nemu {

	// Hack for emscripten
	inline static void CallMain(void* fp)
	{
		std::function<void()>* fn = (std::function<void()>*) fp;
		(*fn)();
	}

	// Emulator setup
	inline static std::unique_ptr<NESInstance> nesInstance, state;
	inline static auto delay = std::chrono::steady_clock::now();
	inline static bool emulatorRunning = false;

	// SDL Stuff
	inline static bool quitting = false;
	inline static SDL_Window* window = NULL;
	inline static SDL_GLContext gl_context = NULL;
	inline static SDL_Renderer* renderer = NULL;
	inline static SDL_Texture* texture = NULL;

	class GUI {
	public:

		static void InitializeEmulator(const NESInstance::Descriptor& descriptor)
		{
			emulatorRunning = true;

			nesInstance = MakeNESInstance(descriptor);

			if (nesInstance == nullptr) {
				std::cout << "Failed to load ROM\n";
				return;
			}
			nesInstance->Power();
		}

		static void HandleEvents(SDL_Event& event)
		{
			if (event.type == SDL_QUIT) {
				quitting = true;
			}
			else if (event.type == SDL_JOYDEVICEADDED) {
				std::cout << "Joystick Connected!" << std::endl;
				if (Input::GetJoystickCount() == 1)
					joystick0 = SDL_JoystickOpen(0);
			}
			else if (event.type == SDL_JOYDEVICEREMOVED) {
				std::cout << "Joystick Disconnected!" << std::endl;
			}
			else if (event.type == SDL_DROPFILE) {
				char* path = event.drop.file;
				SDL_Log(path);

				if (!util::StringEndsWith(path, ".nes")) {
					SDL_free(path);
					return;
				}

				InitializeEmulator(NESInstance::Descriptor{
					ROMLayout(path),
					NESInput(),
					-1
				});

				SDL_free(path);
			}
			else if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.scancode) {
				case SDL_SCANCODE_1: {
					// Copy state
					state = nesInstance->Clone();
				}
				case SDL_SCANCODE_2: {
					// Restore state
					nesInstance = state->Clone();
				}
				}
			}
		}

		static int Run()
		{
			if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK) != 0) {
				SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
				return 1;
			}

			if (TTF_Init() == -1) {
				SDL_Log("TTF_Init: %s\n", TTF_GetError());
				return 1;
			}

			window = SDL_CreateWindow("Nemu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 480, SDL_WINDOW_OPENGL);
			if (window == NULL) {
				SDL_Log("Could not create window: %s\n", SDL_GetError());
				return 1;
			}
			gl_context = SDL_GL_CreateContext(window);
			SDL_GL_MakeCurrent(window, gl_context);

			SDL_JoystickEventState(SDL_ENABLE); // Required to listen for joystick events
			SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 256, 240);
			int pitch;

			std::stringstream fontPath;
			fontPath << SDL_GetBasePath();
			fontPath << "/fonts/OpenSans/OpenSans-Bold.ttf";

			TTF_Font* Sans = TTF_OpenFont(fontPath.str().c_str(), 22); //this opens a font style and sets a size
			if (Sans == NULL) {
				SDL_Log("TTF_OpenFont: %s\n", TTF_GetError());
			}

			SDL_Color White = { 255, 255, 255 };
			SDL_Surface* surfaceMessage = TTF_RenderText_Blended(Sans, "Drop ROM here!", White);
			SDL_Texture* message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
			SDL_Rect Message_rect;
			Message_rect.x = 256 - surfaceMessage->w / 2;
			Message_rect.y = 240 - surfaceMessage->h / 2;
			Message_rect.w = surfaceMessage->w;
			Message_rect.h = surfaceMessage->h;

#ifdef NEMU_PLATFORM_WEB
			std::function<void()> mainLoop = [&]() {
#else
			while (!quitting) {
#endif
				// Poll Events
				SDL_Event event;
				while (SDL_PollEvent(&event))
					HandleEvents(event);

				// Update states
				if (emulatorRunning) {
					nesInstance->RunFrame();
					SDL_UpdateTexture(texture, NULL, nesInstance->GetPixels(), 256 * 4);
				}

				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
				SDL_RenderClear(renderer);

				// Draw
				SDL_RenderCopy(renderer, texture, NULL, NULL);
				if(!emulatorRunning)
					SDL_RenderCopy(renderer, message, NULL, &Message_rect);
				SDL_RenderPresent(renderer);

				// Delay to force 60fps
				std::this_thread::sleep_until(delay);
				delay += std::chrono::nanoseconds(1000000000) / 60;

#ifdef NEMU_PLATFORM_WEB
			};
			emscripten_set_main_loop_arg(CallMain, &mainLoop, 0, 1);
#else
			}

			SDL_JoystickClose(joystick0);
			SDL_DestroyRenderer(renderer);
			SDL_DestroyTexture(texture);
			SDL_DestroyTexture(message);
			TTF_CloseFont(Sans);
			SDL_FreeSurface(surfaceMessage);
			SDL_GL_DeleteContext(gl_context);
			SDL_DestroyWindow(window);
			SDL_Quit();
#endif
		}
	};
}

// Called by javascript to upload a ROM-file to the heap
extern "C" {
	void UploadRom(std::uint8_t* data, std::size_t length)
	{
		nemu::GUI::InitializeEmulator(nemu::NESInstance::Descriptor{
			nemu::ROMLayout(data, length),
			nemu::NESInput(),
			-1
			});
	}
}