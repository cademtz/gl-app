#include <platform.hpp>
#include "render/opengl/em_includes.h"
#include "render/opengl/default_setup.h"

void Platform::Setup() {
    setup_opengl();
}

void Platform::Cleanup() {
    
}