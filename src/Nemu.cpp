//#include "Nemu/PPU.h"
#include "Nemu/CPU.h"
#include "Nemu/NESKeys.h"
#include <Sgl.h>

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
	Renderer2D* renderer;
	Shader shader;

	NESKeyMapper keyMapper;
	std::uint8_t* pixels; // pixels[ x * height * depth + y * depth + z ] = elements[x][y][z]
	Renderable2D frame;
	Texture2D *frameTexture;

public:
	MainLayer()
		: Layer("MainLayer"), shader(VertexShader, FragmentShader)
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

		keyMapper.MapKey(NESKey::Start, SGL_KEY_ENTER);
		keyMapper.MapKey(NESKey::A, SGL_KEY_A);
		keyMapper.MapKey(NESKey::B, SGL_KEY_B);
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

	~MainLayer()
	{
		delete pixels;
		delete frameTexture;
		delete renderer;
	}

	void OnUpdate() override
	{
		/* Update */

		if (Input::IsKeyPressed(keyMapper.GetKey(NESKey::Start)))
			SglInfo("Start key is down!");
		else if (Input::IsKeyPressed(keyMapper.GetKey(NESKey::A)))
			SglInfo("A key is down!");
		else if (Input::IsKeyPressed(keyMapper.GetKey(NESKey::B)))
			SglInfo("B key is down!");

		/* Render */

		renderer->Begin();
		renderer->Submit(frame);
		renderer->SubmitTexture(frameTexture);
		renderer->End();
		renderer->Flush();
	}

	void OnEvent(Event& event) override
	{
		if (event.GetEventType() == EventType::DropEvent) {
			auto& c = (DropEvent&)event;
			SglTrace(c.ToString());
			/* Handle ROM loading */
		}
		else if (event.GetEventType() == EventType::KeyPressed) {
			auto& c = (KeyPressedEvent&)event;
			SglTrace(c.ToString());
		}
		else if (event.GetEventType() == EventType::KeyReleased) {
			auto& c = (KeyReleasedEvent&)event;
			SglTrace(c.ToString());
		}
	}
};

class NESApp : public Application {
public:

	NESApp()
		: Application(Width, Height, "Nemu - NES Emulator")
	{
		PushLayer(new MainLayer());
	}

	~NESApp() {}

};

sgl::Application* sgl::CreateApplication()
{
	return new NESApp;
}
