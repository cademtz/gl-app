/**
 * @file
 * @brief Load and prepare OpenGL to be used
 */

#include <platform.hpp>
#include <app.hpp>
#include <iostream>
#include "opengl.hpp"

#if _IMPL_WINDOW == _IMPL_WINDOW_GLFW
    #include <impl/window/glfw/glfw.hpp>
#endif

static void load_opengl() {
    #ifdef GLAD_GL
        #ifdef GLFW_VERSION_MAJOR // Load with the help of GLFW
            int version = gladLoadGL(glfwGetProcAddress);
            if (!version)
                PLATFORM_ERROR("gladLoadGL failed");
            std::cout << "GL " << GLAD_VERSION_MAJOR(version) << '.' << GLAD_VERSION_MINOR(version);
            std::cout << std::endl;
        #else // Load with GLAD's built-in loader
            // WARNING: Using GLAD's built-in loader is not preferred and may even be unavailable
            gladLoaderLoadGL();
        #endif
    #elif defined(__EMSCRIPTEN__)
        // OpenGL ES is already loaded by default
    #else
        #error No default method to load OpenGL. \
         If this platform loads OpenGL by default, create an empty preprocessor case for it.
    #endif
}

namespace impl::render {

void setup() {
    load_opengl();
    
    // Enable alpha/transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void cleanup() {
}

}
