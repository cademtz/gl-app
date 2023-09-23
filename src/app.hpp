#pragma once
#include <platform.hpp>
#include <render/gui/rendergui.hpp>
#include <memory>

namespace App {

/** Called before running the app */
void OnSetup();
/** Called after running the app */
void OnCleanup();
/** Render to the current frame buffer */
void Render();

};