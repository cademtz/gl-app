#include <platform.hpp>
#include <imgui.h>
#include <render/opengl/setup.hpp>
#include "window/setup.hpp"

void Platform::Setup() {
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    impl::window::setup(1280, 720);
}

void Platform::Cleanup() {
    impl::window::cleanup();
    ImGui::DestroyContext();
}

void Platform::PreRender() {
    impl::window::PreRender();
}

void Platform::PostRender() {
    impl::window::PostRender();
}