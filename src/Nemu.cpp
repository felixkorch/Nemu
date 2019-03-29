#include "Nemu/PPU.h"
#include "Nemu/CPU.h"
#include "Nemu/NESInput.h"
#include "Nemu/NESMemory.h"
#include <ctime>

using namespace sgl;
using namespace nemu;
using namespace ppu;

#define Width 512
#define Height 480

#define TexWidth 256
#define TexHeight 240

using CPUMemoryType = NESMemory<std::uint8_t, NROM256Mapper>;
using CPUType = CPU<CPUMemoryType>;
using PPUType = PPU<CPUType>;

class MainLayer : public Layer {
private:
	static constexpr float aspectRatio = (float)TexWidth / (float)TexHeight;

	std::unique_ptr<Renderer2D> renderer;
	Renderable2D frame;
    Texture2D frameTexture;

	NESInput nesInput;
	CPUMemoryType CPUMemory;
	std::unique_ptr<CPUType> cpu;
	std::unique_ptr<PPUType> ppu;

public:
	MainLayer() :
		Layer("MainLayer"),
		renderer(Renderer2D::Create(Width, Height)),
		frameTexture(TexWidth, TexHeight)
	{
		cpu = sgl::make_unique<CPUType>(CPUMemory);
		ppu = sgl::make_unique<PPUType>(*cpu, std::bind(&MainLayer::OnNewFrame, this, std::placeholders::_1));

		const float scaledWidth = (float)Height * aspectRatio;
		const float xPosition = Width / 2 - scaledWidth / 2;

		frame = Renderable2D(glm::vec2(scaledWidth, Height), glm::vec2(xPosition, 0));

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
	}

	// Callback from the PPU
	void OnNewFrame(std::uint8_t* pixels)
	{
		frameTexture.SetData(pixels);
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
		renderer->SubmitTexture(&frameTexture);
		renderer->Submit(frame);
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

constexpr WindowProperties props{
		Width,                 // WindowWidth
		Height,                // WindowHeight
		"Nemu - NES Emulator", // Title
		false                  // Resizable
};

class NESApp : public Application {
public:

	NESApp()
		: Application(props)
	{
		PushLayer(new MainLayer);
	}

	~NESApp() {}

};

sgl::Application* sgl::CreateApplication()
{
	return new NESApp;
}
