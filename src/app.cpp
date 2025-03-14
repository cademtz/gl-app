#include "app.hpp"
#include <render/font/font.hpp>
#include "glm/ext/scalar_constants.hpp"
#include "platform.hpp"
#include <render/texture.hpp>
#include <render/font/fontmanager.hpp>
#include <glm/glm.hpp>
#include <cmath>
#include <render/opengl/setup.hpp>

// Temporary includes for testing
#include "input/inputqueue.hpp"
#include "render/render2d.hpp"
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

#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include <backends/imgui_impl_opengl3.cpp>

Render2d::Draw draw_gui;
hid::InputQueue input_queue;
static ImGuiID dock_space_id = 0;

void App::OnSetup() {
    OglSetup();
    ImGui_ImplOpenGL3_Init();
    Platform::SetInputHandler(&input_queue);
    Render2d::Setup();
    Dialog::OnSetup();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    font_default = FontManager::CreateFont(FontBakeConfig("Open_Sans/static/OpenSans-Regular.ttf", 32, 3));

    Platform::AddRepeatingTask([] {
        FontManager::RunQueue();
        Platform::PreRender();
        App::Render();
        Platform::PostRender();
        return true;
    });
}

void App::OnCleanup() {
    Render2d::Cleanup();
    FontManager::Cleanup();
    ImGui_ImplOpenGL3_Shutdown();
    OglCleanup();
}

void App::Render() {
    input_queue.Flush([](const hid::Event& e) {
        // TODO: Use this input when necessary.
        // DearImGui's input state suffices at the moment.
    });

    int width, height;
    Platform::GetFrameBufferSize(&width, &height);
    
    draw_gui.Clear();

    // Clear screen with black rect
    draw_gui.SetColor(0,0,0);
    draw_gui.Rect(0, 0, width, height);
    Dialog::OnDrawGui(draw_gui);

    size_t num_drawcalls = draw_gui.GetDrawList().calls.size();
    draw_gui.ResetColor();
    draw_gui.TextAscii(font_default, glm::vec2(32, 50),
        std::to_string(num_drawcalls) + " draw calls\n"
    );

    Render2d::m_screen_w = width;
    Render2d::m_screen_h = height;
    Render2d::render_target = nullptr;
    Render2d::UploadDrawData(draw_gui.GetDrawList());
    Render2d::Render();

    ImGui_ImplOpenGL3_NewFrame();
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
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
