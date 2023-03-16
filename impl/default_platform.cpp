#include <platform.hpp>
#include "render/opengl/glad_includes.h"
#include "render/opengl/default_setup.h"

void Platform::Setup() {
    if (!gladLoadGL(glfwGetProcAddress))
        PLATFORM_ERROR("Failed to initialize OpenGL context");
    
    setup_opengl();
}

void Platform::Cleanup() {
    
}