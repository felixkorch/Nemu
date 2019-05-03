#include <ctime>
#include "Nemu/NESInput.h"
#include "Nemu/NESInstance.h"
#include "Nemu/ROMLayout.h"

using namespace sgl;
using namespace nemu;

#define Width 768
#define Height 720

#define TexWidth 256
#define TexHeight 240

class MainLayer : public Layer {

    static constexpr float aspectRatio = (float)TexWidth / (float)TexHeight;
	Renderer2D renderer;
    Renderable2D frame;
    Texture2D frameTexture;

    std::unique_ptr<NESInstance> nesInstance;
    std::unique_ptr<NESInstance> state;
    NESInput nesInput;
    bool running, showSettings;
	std::chrono::steady_clock::time_point delay;
	int fps;

   public:
    MainLayer()
        : Layer("MainLayer")
		, renderer(Width, Height)
		, frame()
		, frameTexture(TexWidth, TexHeight)
		, nesInput()
		, running(false)
        , showSettings(false)
		, delay(std::chrono::steady_clock::now())
		, fps(60)
	{
        const float scaledWidth = (float)Height * aspectRatio;
        const float xPosition = Width / 2 - scaledWidth / 2;

        frame = Renderable2D(glm::vec2(xPosition, 0), &frameTexture);
		frame.SetSize(scaledWidth, Height);
        frame.SetUV({ glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), glm::vec2(0, 0) });

        // Input - Key Mapping
        NESKeyMapper keyMapper;
        keyMapper.Map(NESButton::Start,  SGL_KEY_ENTER);
        keyMapper.Map(NESButton::Select, SGL_KEY_BACKSPACE);
        keyMapper.Map(NESButton::A,      SGL_KEY_X);
        keyMapper.Map(NESButton::B,      SGL_KEY_Z);
        keyMapper.Map(NESButton::Left,   SGL_KEY_LEFT);
        keyMapper.Map(NESButton::Right,  SGL_KEY_RIGHT);
        keyMapper.Map(NESButton::Up,     SGL_KEY_UP);
        keyMapper.Map(NESButton::Down,   SGL_KEY_DOWN);
        nesInput.AddKeyboardConfig(keyMapper);

        // Input - Joystick Mapping
        AxisConfig left {0, AxisConfig::Value::Negative};
        AxisConfig right{0, AxisConfig::Value::Positive};
        AxisConfig up   {1, AxisConfig::Value::Negative};
        AxisConfig down {1, AxisConfig::Value::Positive};

        NESJoystickMapper joystickMapper;
        joystickMapper.MapKey (NESButton::A, 0);
        joystickMapper.MapAxis(NESButton::Left, left);
        joystickMapper.MapAxis(NESButton::Right, right);
        joystickMapper.MapAxis(NESButton::Up, up);
        joystickMapper.MapAxis(NESButton::Down, down);
        nesInput.AddJoystickConfig(joystickMapper);
    }

    ~MainLayer() override
	{}

    // Callback from the PPU
    void OnNewFrame(std::uint8_t* pixels)
	{
        frameTexture.SetData(pixels);
    }

    void OnUpdate() override
	{
        // Update
        if (running)
            nesInstance->RunFrame();

        // Render
		renderer.Begin();
		renderer.Submit(&frame);
		renderer.End();
		renderer.Flush();

		std::this_thread::sleep_until(delay);
		delay += std::chrono::nanoseconds(1000000000) / fps;
    }

    void OnEvent(Event& event) override
	{
        // Rom Loading
        if (event.GetEventType() == EventType::DropEvent) {
            auto& e = (DropEvent&)event;
            SGL_TRACE(e.ToString());

            auto paths = e.GetPaths();
            if (paths.size() > 1)
                return;

            if (!EndsWith(paths[0], ".nes"))
                return;

            running = true;

            nesInstance = MakeNESInstance((NESInstance::Descriptor{
                ROMLayout(paths[0]),
                nesInput,
                std::bind(&MainLayer::OnNewFrame, this, std::placeholders::_1),
                -1
            }));
            /*
            nesInstance = MakeNESInstance(
                paths[0],
                nesInput,
                std::bind(&MainLayer::OnNewFrame, this, std::placeholders::_1));
            */
            if (nesInstance == nullptr) {
                std::cout << "Failed to load ROM\n";
                return;
            }
            nesInstance->Power();
        }
        // Window Resized
        else if (event.GetEventType() == EventType::WindowResized) {
            auto& e = (WindowResizedEvent&)event;
            // Width according to aspect ratio
            const float scaledWidth = (float)e.GetHeight() * aspectRatio;
            // Centralize the frame
            const float xPosition = e.GetWidth() / 2 - scaledWidth / 2;
            // Update the renderable
            frame = Renderable2D(glm::vec2(scaledWidth, e.GetHeight()),
                                 glm::vec2(xPosition, 0));
			frame.SetUV({ glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), glm::vec2(0, 0) });
        }

		else if (event.GetEventType() == EventType::KeyPressed) {
			KeyPressedEvent& e = (KeyPressedEvent&)event;
			// TODO: Load / Save State

            if(e.GetKeyCode() == SGL_KEY_ESCAPE)
                showSettings = !showSettings;
		}
    }

    void OnImGuiRender() override
    {
        auto& app = Application::Get();
        static int counter = 0;

        //ImGui::ShowDemoWindow();

        if (showSettings) {
            bool show = ImGui::Begin("Settings", &showSettings, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
            if (!show) {
                ImGui::End();
            }
            else {
                ImGui::SetWindowSize(ImVec2((float)app.GetWindow()->GetWidth() * 0.75, (float)app.GetWindow()->GetHeight() * 0.75));
                ImGui::SetWindowPos(ImVec2((float)app.GetWindow()->GetWidth() * 0.125, (float)app.GetWindow()->GetHeight() * 0.125));

                ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None);
                if (ImGui::BeginTabItem("Emulator")) {
                    ImGui::NewLine();
                   /* ImGui::Columns(2, "Columns", false);*/
                    //ImGui::Separator();

                    ImGui::Text("Set Frames per second.");
                    ImGui::SameLine();
                    ImGui::SliderInt("FPS", &fps, 30, 144);
                    //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

                    /*ImGui::NextColumn();

                    if (ImGui::Button("Increase Counter"))
                        counter++;
                    ImGui::SameLine();
                    ImGui::Text("counter = %d", counter);*/
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Input")) {
                    ImGui::PushItemWidth(90);
                    ImGui::NewLine();
                    if (ImGui::Button("A", ImVec2(60, 30))) SGL_TRACE("Awaiting input...");
                    ImGui::SameLine();
                    ImGui::Text("Keyboard: X");
                    ImGui::NewLine();

                    if (ImGui::Button("B", ImVec2(60, 30))) SGL_TRACE("Awaiting input...");
                    ImGui::SameLine();
                    ImGui::Text("Keyboard: X");
                    ImGui::NewLine();

                    if (ImGui::Button("UP", ImVec2(60, 30))) SGL_TRACE("Awaiting input...");
                    ImGui::SameLine();
                    ImGui::Text("Keyboard: X");
                    ImGui::NewLine();

                    if (ImGui::Button("DOWN", ImVec2(60, 30))) SGL_TRACE("Awaiting input...");
                    ImGui::SameLine();
                    ImGui::Text("Keyboard: X");
                    ImGui::NewLine();

                    if (ImGui::Button("LEFT", ImVec2(60, 30))) SGL_TRACE("Awaiting input...");
                    ImGui::SameLine();
                    char name[20] = "Keyboard: X";
                    ImGui::InputText("##edit", name, IM_ARRAYSIZE(name), ImGuiInputTextFlags_ReadOnly, 0);
                    ImGui::NewLine();

                    if (ImGui::Button("RIGHT", ImVec2(60, 30))) SGL_TRACE("Awaiting input...");
                    ImGui::SameLine();
                    ImGui::Text("Keyboard: X");

                    ImGui::PopItemWidth();

                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();

                ImGui::End();
            }
        }
    }

private:
    bool EndsWith(const std::string& str, const std::string& suffix)
    {
        return str.size() >= suffix.size() &&
            str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
};

const WindowProperties props {
    Width,                 // WindowWidth
    Height,                // WindowHeight
    false,                  // Resizable
    "Nemu - NES Emulator"  // Title
};

class NESApp : public sgl::Application {
public:
	NESApp()
		: sgl::Application(props)
	{
        window->SetVSync(false);
        PushLayer(new MainLayer);
    }

    ~NESApp() {}
};

sgl::Application* sgl::CreateApplication()
{
    return new NESApp;
}
