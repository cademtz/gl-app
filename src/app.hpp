#pragma once
#include <platform.hpp>
#include <render/gui/forward.hpp>
#include <memory>
#include <imgui.h>

namespace App {

inline ImGuiID imgui_dock_left = 0;
inline gui::FontHandle font_default;

/** Called before running the app */
void OnSetup();
/** Called after running the app */
void OnCleanup();
/** Render to the current frame buffer */
void Render();

};