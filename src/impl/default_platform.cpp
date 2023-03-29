#include <platform.hpp>
#include "render/setup.hpp"
#include "window/setup.hpp"

void Platform::Setup() {
    _setup_window_api();
    _setup_render_api();
}

void Platform::Cleanup() {
    _cleanup_render_api();
    _cleanup_window_api();
}