#include "Nemu/PPU.h"
#include "Nemu/CPU.h"
#include "Nemu/NESInput.h"
#include <ctime>

using namespace sgl;
using namespace nemu;

#define VertexShader   Shader::Core_Vertex_Shader2D
#define FragmentShader Shader::Core_Fragment_Shader2D

#define Width 1280
#define Height 720

#define TexWidth 256
#define TexHeight 240


class MainLayer : public Layer {
private:
	using CPUMemoryType = VectorMemory<std::uint8_t>;
	using PPUMemoryType = ppu::PPUInternalMem;

	const float aspectRatio = (float)TexWidth / (float)TexHeight;
	Renderer2D* renderer;
	Shader shader;
	Renderable2D frame;
    Texture2D* frameTexture;

	NESInput nesInput;

	CPUMemoryType CPUMemory;
	CPU<CPUMemoryType>* cpu;
	ppu::PPU<CPUMemoryType, PPUMemoryType>* ppu;

public:
	MainLayer()
		: Layer("MainLayer"), shader(VertexShader, FragmentShader)
	{
		cpu = new CPU<CPUMemoryType>(CPUMemory);
		ppu = new ppu::PPU<CPUMemoryType, PPUMemoryType>(CPUMemory, std::bind(&MainLayer::OnNewFrame, this, std::placeholders::_1));

		renderer = Renderer2D::Create(Width, Height, shader);

		const float scaledWidth = (float)Height * aspectRatio;
		const float xPosition = Width / 2 - scaledWidth / 2;

		frame = Renderable2D(glm::vec2(scaledWidth, Height), glm::vec2(xPosition, 0));
		frameTexture = new Texture2D(TexWidth, TexHeight);

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

    ~MainLayer() override
	{
		delete frameTexture;
		delete renderer;
		delete cpu;
		delete ppu;
	}

	// Callback from the PPU
	void OnNewFrame(std::uint8_t* pixels)
	{
		frameTexture->SetData(pixels);
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
		}
		else if (event.GetEventType() == EventType::KeyReleased) {
			auto& e = (KeyReleasedEvent&)event;
			SglTrace(e.ToString());
		}
		else if (event.GetEventType() == EventType::WindowResizedEvent) {
			auto& e = (WindowResizedEvent&)event;
			// Width according to aspect ratio
			const float scaledWidth = (float)e.GetHeight() * aspectRatio;
			// Centralize the frame
			const float xPosition = e.GetWidth() / 2 - scaledWidth / 2;
			// Update the renderable
			frame = Renderable2D(glm::vec2(scaledWidth, e.GetHeight()), glm::vec2(xPosition, 0));
			// Set size of camera
			renderer->SetScreenSize(e.GetWidth(), e.GetHeight());
		}
	}
};

class NESApp : public Application {
public:

	NESApp()
		: Application(Width, Height, "Nemu - NES Emulator")
	{
		PushLayer(new MainLayer);
	}

	~NESApp() {}

};

sgl::Application* sgl::CreateApplication()
{
	return new NESApp;
}


/*
glm::vec4 HexToRgb(unsigned int hexValue)
{
	glm::vec4 rgbColor;

	rgbColor.x = ((hexValue >> 16) & 0xFF);
	rgbColor.y = ((hexValue >> 8) & 0xFF);
	rgbColor.z = ((hexValue) & 0xFF);
	rgbColor.w = 1;

	return rgbColor;
}*/

/*
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
}*/