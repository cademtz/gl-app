#include "app.hpp"
#include "font.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "platform.hpp"
#include <render/texture.hpp>
#include <font.hpp>
#include <render/gui/draw.hpp>
#include <render/gui/fontmanager.hpp>
#include <glm/glm.hpp>
#include <cmath>

// Temporary includes for testing
#include "demostuff.hpp"
#include "input/inputqueue.hpp"
#include "render/gui/rendergui.hpp"
#include "render/sticks/draw.hpp"
#include "render/sticks/drawlist.hpp"
#include "render/sticks/rendersticks.hpp"
#include <chrono>
#include <string>
#include <vector>
#include <resources/resource.hpp>
#include <string_view>
#include <iostream>
#include <cstdlib>
#include <cstring>

gui::Draw draw_gui;
gui::FontHandle default_font = gui::FontManager::CreateFont(FontBakeConfig("Open_Sans/static/OpenSans-Regular.ttf", 32, 3));
std::unique_ptr<DemoStuff> demostuff = std::make_unique<DemoStuff>();
hid::InputQueue input_queue;

void App::OnSetup() {
    Platform::SetInputHandler(&input_queue);

    Platform::AddRepeatingTask([] {
        gui::FontManager::RunQueue();
        Platform::PreRender();
        App::Render();
        Platform::PostRender();
        return true;
    });
}

void App::OnCleanup() {
    gui::FontManager::Cleanup();
}

void App::Render() {
    input_queue.Flush([](const hid::Event& e) {
        demostuff->RunEvent(e);
    });

    int width, height;
    Platform::GetFrameBufferSize(&width, &height);
    
    draw_gui.Clear();

    // Clear screen with black rect
    draw_gui.SetColor(0,0,0);
    draw_gui.Rect(0, 0, width, height);

    demostuff->DrawGui(draw_gui);

    size_t num_drawcalls = draw_gui.GetDrawList().calls.size();
    draw_gui.ResetColor();
    draw_gui.TextAscii(default_font, glm::vec2(32, 50),
        std::to_string(num_drawcalls) + " draw calls\n"
    );

    static auto draw_sticks = sticks::Draw();

    draw_sticks.Clear();
    demostuff->DrawSticks(draw_sticks);
    static auto render_sticks = sticks::RenderSticks::GetInstance();

    static std::shared_ptr<gui::RenderGui> render_gui = gui::RenderGui::GetInstance();
    render_gui->SetScreenSize(width, height);
    render_gui->UploadDrawData(draw_gui.GetDrawList());
    render_gui->Render();

    render_sticks->UploadDrawData(draw_sticks.GetDrawList());
    render_sticks->Render();
}
