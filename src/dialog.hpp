#pragma once
#include <render/gui/forward.hpp>

namespace Dialog {

void OnSetup();
void OnImGuiDock();
void OnImGui();
void OnImGuiViewport(bool viewport_input);
void OnDrawGui(gui::Draw& draw);
void OnInput();

}