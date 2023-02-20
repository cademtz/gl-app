// Includes all features to interact with the current platform
#pragma once
#ifdef __EMSCRIPTEN__
#   include <emscripten.h>
#   define GL_GLEXT_PROTOTYPES
#   define EGL_EGLEXT_PROTOTYPES
#else
#   include <glad/gl.h>
#endif

#include <GLFW/glfw3.h>

#define PLATFORM_WARNING(Msg) Platform::Warning(Msg, __FILE__, __LINE__)
#define PLATFORM_ERROR(Msg) Platform::Error(Msg, __FILE__, __LINE__)

namespace Platform {
    void Warning(const char* Msg, const char* File = 0, int Line = -1);
    void Error(const char* Msg, const char* File = 0, int Line = -1);
}