/**
 * @brief Load and prepare OpenGL to be used
 * 
 */

#include <platform.hpp>
#include <app.hpp>
#include <iostream>
#include "opengl.hpp"
#include "renderguiglsl.hpp"

#if _IMPL_WINDOW == _IMPL_WINDOW_GLFW
    #include <impl/window/glfw/glfw.hpp>
#endif

static void load_opengl()
{
    #if defined(GLAD_GL)
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
         If this platform loads OpenGL by default, create a dummy preprocessor case for it.
    #endif
}

void _setup_render_api()
{
    load_opengl();
    
    // Enable alpha/transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    App::SetGuiRenderer(std::make_unique<CRenderGuiGlsl>());
}

void _cleanup_render_api()
{
    // Assign nullptr to destroy the previous values
    App::SetGuiRenderer(nullptr);
}