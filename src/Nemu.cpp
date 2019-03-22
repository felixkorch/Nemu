#include "Nemu/PPU.h"
#include "Nemu/CPU.h"
#include "Nemu/NESInput.h"
#include <ctime>

#define VertexShader   Shader::Core_Vertex_Shader2D
#define FragmentShader Shader::Core_Fragment_Shader2D

#define Width 512
#define Height 480

#define TexWidth 256
#define TexHeight 240

using namespace sgl;
using namespace nemu;

class MainLayer : public Layer {
private:
	Window& window;
	Renderer2D* renderer;
	Shader shader;
	std::uint8_t* pixels; // pixels[ x * height * depth + y * depth + z ] = elements[x][y][z]
	Renderable2D frame;
    Texture2D* frameTexture;

	NESInput nesInput;
	// PPU ppu;
	// CPU cpu;

public:
	MainLayer(Window& window)
		: Layer("MainLayer"), shader(VertexShader, FragmentShader), window(window)
	{
		renderer = Renderer2D::Create(Width, Height, shader);
		frame = Renderable2D(glm::vec2(Width, Height), glm::vec2(0, 0));
		frameTexture = new Texture2D(TexWidth, TexHeight);

		srand(time(nullptr));
		pixels = new std::uint8_t[TexWidth * TexHeight * 4];
		
		int i, j;

		for (i = 0; i < TexWidth; i++) {
			for (j = 0; j < TexHeight; j++) {
				auto c = HexToRgb(ppu::nesRGB[rand() % 64]);
				pixels[i * TexHeight * 4 + j * 4 + 0] = (std::uint8_t)c.x;
				pixels[i * TexHeight * 4 + j * 4 + 1] = (std::uint8_t)c.y;
				pixels[i * TexHeight * 4 + j * 4 + 2] = (std::uint8_t)c.z;
				pixels[i * TexHeight * 4 + j * 4 + 3] = (std::uint8_t)255;
			}
		}

		frameTexture->SetData(pixels);

		/* Input */
		NESKeyMapper keyMapper;
		keyMapper.Map(NESButton::Start, SGL_KEY_ENTER);
		keyMapper.Map(NESButton::A,     SGL_KEY_A);
		keyMapper.Map(NESButton::B,     SGL_KEY_B);
		keyMapper.Map(NESButton::Left,  SGL_KEY_LEFT);
		keyMapper.Map(NESButton::Right, SGL_KEY_RIGHT);
		nesInput.AddKeyboardConfig(keyMapper);

		NESJoystickMapper joystickMapper;
		joystickMapper.MapKey(NESButton::A, 0);
		nesInput.AddJoystickConfig(joystickMapper);

	}

	glm::vec4 HexToRgb(unsigned int hexValue)
	{
		glm::vec4 rgbColor;

		rgbColor.x = ((hexValue >> 16) & 0xFF);
		rgbColor.y = ((hexValue >> 8) & 0xFF);
		rgbColor.z = ((hexValue) & 0xFF);
		rgbColor.w = 1;

		return rgbColor;
	}

    ~MainLayer() override
	{
		delete pixels;
		delete frameTexture;
		delete renderer;
	}

	// Logic to scale the frame when entering fullscreen
	void ToggleFullScreen()
	{
		window.ToggleFullScreen();
		int newWidth = (float)window.GetWindowHeight() * frame.bounds.size.x / frame.bounds.size.y;
		int newHeight = window.GetWindowHeight();
		frame = Renderable2D(glm::vec2(newWidth, newHeight), glm::vec2(0, 0));
		frame.bounds.pos.x = window.GetWindowWidth() / 2 - frame.bounds.size.x / 2; // Centralize the texture
		frameTexture->SetSize(frame.bounds.size.x, frame.bounds.size.y);
		frameTexture->SetData(pixels);
		renderer->SetScreenSize(window.GetWindowWidth(), window.GetWindowHeight());
	}

	void OnUpdate() override
	{
		/* Update */

		if (nesInput.Get(NESButton::Start))
			SglInfo("Start key is down!");
		else if (nesInput.Get(NESButton::A))
			SglInfo("A key is down!");
		else if (nesInput.Get(NESButton::B))
			SglInfo("B key is down!");

		/* Render */

		auto newFrame = nullptr; // ppu.NewFrame();
		if(newFrame)
			frameTexture->SetData(newFrame);

		renderer->Begin();
		renderer->Submit(frame);
		renderer->SubmitTexture(frameTexture);
		renderer->End();
		renderer->Flush();
	}

	void OnEvent(Event& event) override
	{
		if (event.GetEventType() == EventType::DropEvent) {
			auto& e = (DropEvent&)event;
			SglTrace(e.ToString());
			/* Handle ROM loading */
		}
		else if (event.GetEventType() == EventType::KeyPressed) {
			auto& e = (KeyPressedEvent&)event;
			SglTrace(e.ToString());

			// Fullscreen (Alt-Enter)
			if (e.GetKeyCode() == SGL_KEY_ENTER && Input::IsKeyPressed(SGL_KEY_LEFT_ALT))
				ToggleFullScreen();
		}
		else if (event.GetEventType() == EventType::KeyReleased) {
			auto& e = (KeyReleasedEvent&)event;
			SglTrace(e.ToString());
		}
	}
};

class NESApp : public Application {
public:

	NESApp()
		: Application(Width, Height, "Nemu - NES Emulator")
	{
		PushLayer(new MainLayer(*window));
	}

	~NESApp() {}

};

sgl::Application* sgl::CreateApplication()
{
	return new NESApp;
}
