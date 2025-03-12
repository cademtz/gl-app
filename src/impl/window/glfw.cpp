// Ensure OpenGL is always included before GLFW
#include <render/opengl/opengl.hpp>
#include <GLFW/glfw3.h>
#include <input/inputhandler.hpp>
#include <platform.hpp>
#include <cstdlib>
#include <backends/imgui_impl_glfw.cpp>

static GLFWwindow* window = nullptr;
static hid::InputHandler* handler = nullptr;

static void error_callback(int error, const char* description);
static bool MainTask();
static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CharCallback(GLFWwindow* window, uint32_t codepoint);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void CursorEnterCallback(GLFWwindow* window, int entered);
static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

namespace impl::window {

using ::window;

void setup(int width, int height) {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(width, height, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, &KeyCallback);
    glfwSetCharCallback(window, &CharCallback);
    glfwSetCursorPosCallback(window, &CursorPosCallback);
    glfwSetCursorEnterCallback(window, &CursorEnterCallback);
    glfwSetMouseButtonCallback(window, &MouseButtonCallback);
    glfwSetScrollCallback(window, &ScrollCallback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    Platform::AddRepeatingTask(&MainTask);
}

void cleanup() {
    ImGui_ImplGlfw_Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

void PreRender() {
    ImGui_ImplGlfw_NewFrame();
}
void PostRender() {
    glfwSwapBuffers(window);
}

}

namespace Platform {
    void GetFrameBufferSize(int* w, int* h) { glfwGetFramebufferSize(window, w, h); }
    void SetInputHandler(hid::InputHandler* h) { handler = h; }
}

static bool MainTask() {
    // TODO: GLFW docs says event processing is normally done after buffer swapping,
    // but this task will run before all others (including rendering)
    // https://www.glfw.org/docs/3.3/input_guide.html#events
    glfwPollEvents();
    if (glfwWindowShouldClose(window))
        Platform::SetShouldClose();
    return true;
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (handler) {
        uint8_t actkshion = ~(uint8_t)0;
        switch (action) {
        case GLFW_PRESS: actkshion = hid::KEY_ACTION_PRESS; break;
        case GLFW_REPEAT: actkshion = hid::KEY_ACTION_REPEAT; break;
        case GLFW_RELEASE: actkshion = hid::KEY_ACTION_RELEASE; break;
        }
        hid::PhysicalKey physicalKey = { };
        physicalKey.code = (hid::KeyCode)key;
        physicalKey.action = (hid::KeyAction)actkshion;
        handler->RunEvent(physicalKey);
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}
static void CharCallback(GLFWwindow* window, uint32_t codepoint) {
    if (handler)
        handler->RunEvent(hid::CharacterKey{codepoint});
}
static void CursorPosCallback(GLFWwindow* window, double x, double y) {
    if (handler)
        handler->RunEvent(hid::MousePos{x, y});
}
static void CursorEnterCallback(GLFWwindow* window, int entered) { }
static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (handler) {
        hid::MouseButton btn;
        switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT: btn.code = hid::MOUSE_LEFT; break;
        case GLFW_MOUSE_BUTTON_RIGHT: btn.code = hid::MOUSE_RIGHT; break;
        case GLFW_MOUSE_BUTTON_MIDDLE: btn.code = hid::MOUSE_MIDDLE; break;
        default:
            if (button > 255)
                btn.code = hid::MOUSE_UNKNOWN;
            else
                btn.code = (hid::MouseCode)button;
        }
        switch (action) {
        case GLFW_PRESS: btn.action = hid::MOUSE_ACTION_PRESS; break;
        case GLFW_RELEASE: btn.action = hid::MOUSE_ACTION_RELEASE; break;
        default:
            return;
        }
        handler->RunEvent(btn);
    }
}
static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (handler)
        handler->RunEvent(hid::Scroll{xoffset, yoffset});
}

static void error_callback(int error, const char* description) {
    PLATFORM_ERROR(description);
}