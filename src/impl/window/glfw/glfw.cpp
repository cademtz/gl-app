#include "glfw.hpp"
#include <platform.hpp>
#include <cstdlib>

namespace impl::glfw {
    GLFWwindow* window = nullptr;
    static bool MainTask()
    {
        // FIXME: GLFW docs says event processing is normally done after buffer swapping,
        // but this task will run before all others (including rendering)
        // https://www.glfw.org/docs/3.3/input_guide.html#events
        glfwPollEvents();
        if (glfwWindowShouldClose(window))
            Platform::SetShouldClose();
        return true;
    }

    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

using namespace impl::glfw;

static void error_callback(int error, const char* description);

void _setup_window_api() {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, KeyCallback);
    //glfwSetCursorPosCallback(window, cursor_callback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    Platform::AddRepeatingTask(&MainTask);
}

void _cleanup_window_api() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

namespace Platform {
    void GetFrameBufferSize(int* w, int* h) {
        glfwGetFramebufferSize(window, w, h);
    }
}

static void error_callback(int error, const char* description) {
    PLATFORM_ERROR(description);
}