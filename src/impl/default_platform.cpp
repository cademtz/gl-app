#include <platform.hpp>
#include "render/setup.hpp"
#include "window/setup.hpp"

void Platform::Setup() {
    impl::window::setup();
    impl::render::setup();
}

void Platform::Cleanup() {
    impl::render::cleanup();
    impl::window::cleanup();
}