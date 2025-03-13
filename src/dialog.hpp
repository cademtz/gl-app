#pragma once
#include <render/forward.hpp>

namespace Dialog {

void OnSetup();
void OnImGuiDock();
void OnImGui();
void OnImGuiViewport(bool viewport_input);
void OnDrawGui(Render2d::Draw& draw);
void OnInput();

}