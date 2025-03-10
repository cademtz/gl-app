#include <platform.hpp>
#include <imgui.h>
#include "render/setup.hpp"
#include "window/setup.hpp"

void Platform::Setup() {
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    impl::window::setup();
    impl::render::setup();
}

void Platform::Cleanup() {
    impl::render::cleanup();
    impl::window::cleanup();
    ImGui::DestroyContext();
}

void Platform::PreRender() {
    impl::window::PreRender();
    impl::render::PreRender();
}

void Platform::PostRender() {
    impl::render::PostRender();
    impl::window::PostRender();
}