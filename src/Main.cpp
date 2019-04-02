#include "Nemu/PPU/PPU.h"
#include "Nemu/CPU/CPU.h"
#include "Nemu/NESInput.h"
#include "Nemu/NESMemory.h"
#include <ctime>

using namespace sgl;
using namespace nemu;

#define Width 512
#define Height 480

#define TexWidth 256
#define TexHeight 240

class MainLayer : public Layer {
private:
	static constexpr float aspectRatio = (float)TexWidth / (float)TexHeight;
	std::unique_ptr<Renderer2D> renderer;
	Renderable2D frame;
    Texture2D frameTexture;

	Nemu nemu;
	NESInput nesInput;
	bool running;

public:
	MainLayer() :
		Layer("MainLayer"),
		renderer(Renderer2D::Create(Width, Height)),
		frame(),
		frameTexture(TexWidth, TexHeight),
		nesInput(),
		running(false)
	{
		const float scaledWidth = (float)Height * aspectRatio;
		const float xPosition = Width / 2 - scaledWidth / 2;

		frame = Renderable2D(glm::vec2(scaledWidth, Height), glm::vec2(xPosition, 0));
		frame.uv = { glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), glm::vec2(0, 0) };

		// Input
		NESKeyMapper keyMapper;
		keyMapper.Map(NESButton::Start,  SGL_KEY_ENTER);
		keyMapper.Map(NESButton::Select, SGL_KEY_BACKSPACE);
		keyMapper.Map(NESButton::A,      SGL_KEY_A);
		keyMapper.Map(NESButton::B,      SGL_KEY_S);
		keyMapper.Map(NESButton::Left,   SGL_KEY_LEFT);
		keyMapper.Map(NESButton::Right,  SGL_KEY_RIGHT);
		keyMapper.Map(NESButton::Up,     SGL_KEY_UP);
		keyMapper.Map(NESButton::Down,   SGL_KEY_DOWN);
		nesInput.AddKeyboardConfig(keyMapper);

		AxisConfig left { 0, AxisConfig::Value::Negative };
		AxisConfig right{ 0, AxisConfig::Value::Positive };
		AxisConfig up   { 1, AxisConfig::Value::Negative };
		AxisConfig down { 1, AxisConfig::Value::Positive };
		NESJoystickMapper joystickMapper;
		joystickMapper.MapKey(NESButton::A, 0);
		joystickMapper.MapAxis(NESButton::Left, left);
		joystickMapper.MapAxis(NESButton::Right, right);
		joystickMapper.MapAxis(NESButton::Up, up);
		joystickMapper.MapAxis(NESButton::Down, down);
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
		// Update
		if (running) nemu.RunFrame();

		// Render
		renderer->Begin();
		renderer->SubmitTexture(&frameTexture);
		renderer->Submit(frame);
		renderer->End();
		renderer->Flush();
	}

	void OnEvent(Event& event) override
	{
		// Handling Rom-Loading
		if (event.GetEventType() == EventType::DropEvent) {
			auto& e = (DropEvent&)event;
			SglTrace(e.ToString());

			auto paths = e.GetPaths();
			if (paths.size() > 1)
				return;

			running = true;

			Joypad joypad;
			joypad.AddInputConfig(nesInput);

			nemu = Nemu();
			nemu.AddJoypad(joypad);
			nemu.CreateCPU<CPU>();
			nemu.CreatePPU<PPU>(std::bind(&MainLayer::OnNewFrame, this, std::placeholders::_1));
			nemu.LoadROM(paths[0]);
			nemu.Power();

		}
		// Window Resized
		else if (event.GetEventType() == EventType::WindowResizedEvent) {
			auto& e = (WindowResizedEvent&)event;
			// Width according to aspect ratio
			const float scaledWidth = (float)e.GetHeight() * aspectRatio;
			// Centralize the frame
			const float xPosition = e.GetWidth() / 2 - scaledWidth / 2;
			// Update the renderable
			frame = Renderable2D(glm::vec2(scaledWidth, e.GetHeight()), glm::vec2(xPosition, 0));
			frame.uv = { glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), glm::vec2(0, 0) };
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
