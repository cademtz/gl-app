#include <platform.h>
#include <app.h>
#include <render/glsl/renderguiglsl.h>

static void error_callback(int error, const char *description);
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void setup_glfw();
static void setup_opengl();

extern "C" int main(int argc, char** argv)
{
    setup_glfw();
    setup_opengl();

    App::OnStartup();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(&App::Loop, 0, true);
#else
    while (!glfwWindowShouldClose(App::GetWindow()))
        App::Loop();
#endif

    App::OnClose();

    glfwDestroyWindow(App::GetWindow());
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

static void setup_glfw()
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    App::SetWindow(window);
}

static void setup_opengl()
{
#ifndef __EMSCRIPTEN__
    if (!gladLoadGL(glfwGetProcAddress))
        PLATFORM_ERROR("Failed to initialize OpenGL context");
#endif

    // Enable alpha/transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    App::SetGuiRenderer(std::make_unique<CRenderGuiGlsl>());
}

static void error_callback(int error, const char *description) {
    PLATFORM_ERROR(description);
}
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    App::KeyCallback(window, key, scancode, action, mods);
}
