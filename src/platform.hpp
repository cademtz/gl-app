/**
 * @file platform.hpp
 * @brief Includes all features to interact with the platform
 *
 */

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

    /**
     * @brief Prepare the application to run on its current platform.
     * This means initializing any necessary APIs, windows, input callbacks, and more.
     */
    void Setup();

    /**
     * @brief Perform necessary cleanup, if any, before the application exits.
     */
    void Cleanup();

    /**
     * @brief Repeat the application logic forever.
     * Manage threading between calls if necessary.
     *
     * @param function Logic to be repeated
     */
    void Loop(void(*function)());
}