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
#include <imgui_internal.h>
#include "dialog.hpp"

gui::Draw draw_gui;
std::unique_ptr<DemoStuff> demostuff = std::make_unique<DemoStuff>();
hid::InputQueue input_queue;
static ImGuiID dock_space_id = 0;

void App::OnSetup() {
    Platform::SetInputHandler(&input_queue);
    Dialog::OnSetup();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    font_default = gui::FontManager::CreateFont(FontBakeConfig("Open_Sans/static/OpenSans-Regular.ttf", 32, 3));

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
    draw_gui.TextAscii(font_default, glm::vec2(32, 50),
        std::to_string(num_drawcalls) + " draw calls\n"
    );
    Dialog::OnDrawGui(draw_gui);

    static auto draw_sticks = sticks::Draw();

    draw_sticks.Clear();
    demostuff->DrawSticks(draw_sticks);
    static auto render_sticks = sticks::RenderSticks::GetInstance();

    static std::shared_ptr<gui::RenderGui> render_gui = gui::RenderGui::GetInstance();
    render_gui->SetScreenSize(width, height);
    render_gui->UploadDrawData(draw_gui.GetDrawList());
    render_gui->SetTarget(nullptr);
    render_gui->Render();

    render_sticks->UploadDrawData(draw_sticks.GetDrawList());
    render_sticks->Render();

    ImGui::NewFrame();
    dock_space_id = ImGui::GetID("root");
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    static bool is_docking_setup = false;
    if (!is_docking_setup) {
        is_docking_setup = true;
        ImGuiID imgui_dock_center;
        ImGui::DockBuilderAddNode(dock_space_id);
        ImGui::DockBuilderSetNodeSize(dock_space_id, viewport->Size);
        ImGui::DockBuilderSplitNode(dock_space_id, ImGuiDir_Left, 0.2f, &imgui_dock_left, &imgui_dock_center);
        Dialog::OnImGuiDock();
        ImGui::DockBuilderDockWindow("Viewport", imgui_dock_center);
        ImGui::DockBuilderFinish(dock_space_id);
    }
    
    ImGui::SetNextWindowPos({0.f,0.f});
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    const ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground
    | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
    | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Editor", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGui::DockSpace(dock_space_id);
    Dialog::OnImGui();
    if (ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoBackground))
    {
        bool viewport_input = ImGui::IsWindowHovered();
        Dialog::OnImGuiViewport(viewport_input);
    }
    ImGui::End();
    
    ImGui::End();

    ImGui::Render();
}
