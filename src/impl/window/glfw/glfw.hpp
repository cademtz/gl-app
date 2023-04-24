#pragma once

// Ensure OpenGL is always included before GLFW
#if _IMPL_RENDER == _IMPL_RENDER_OPENGL
    #include <impl/render/opengl/opengl.hpp>
#endif

#include <GLFW/glfw3.h>

namespace impl::glfw
{
    extern GLFWwindow *window;
}