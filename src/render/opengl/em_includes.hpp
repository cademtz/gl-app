#pragma once
#include <emscripten.h>

#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES

// Emscripten puts all the OpenGL header data in GLFW... for some reason
#if _WINDOW_IMPL == _WINDOW_IMPL_GLFW
    #include <GLFW/glfw3.h>
#endif