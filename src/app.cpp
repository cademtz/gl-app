#include "app.hpp"
#include <render/geometry.hpp>
#include <stb_truetype.h>

static GLFWwindow* window = nullptr;
static std::unique_ptr<CRenderGui> guiRenderer = nullptr;
static const Geometry2d base_geom = {
    // Vertices
    { //  x y       u v     r g b a
        { 100,100,    0,0,    1,0,0,1 },
        { 100,200,    0,10,   0,1,0,1 },
        { 200,200,    10,10,  0,0,1,1 },
        { 200,100,    10,0,   1,1,1,1 },
    },
    // Indices
    {
        0, 1, 2,    2, 3, 0,
    }
};

Geometry2d geom_buf;

void App::OnStartup()
{
    guiRenderer->Init();

    glClearColor(0, 0, 0, 1);
}

void App::OnClose() {
    guiRenderer = nullptr;
}

void App::Loop()
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    guiRenderer->SetScreenSize(width, height);

    geom_buf.vertices.clear();
    geom_buf.indices.clear();

    geom_buf.vertices.insert(geom_buf.vertices.begin(), base_geom.vertices.begin(), base_geom.vertices.end());
    geom_buf.indices.insert(geom_buf.indices.begin(), base_geom.indices.begin(), base_geom.indices.end());

    PrimitiveBuilder2d shapes(geom_buf);

    shapes.SetAlign(PrimitiveBuilder2d::Align_Center);
    shapes.SetRotation(glfwGetTime());
    shapes.SetColor(0, 1, 1);
    shapes.Rectangle(400, 400, 200, 50);
    shapes.SetColor(1, 0, 0);
    shapes.Ellipse(400, 400, 200, 50);

    guiRenderer->UploadGeometry(geom_buf);

    glClear(GL_COLOR_BUFFER_BIT);
    guiRenderer->Render();

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void App::KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void App::CursorCallback(GLFWwindow* window, double xpos, double ypos)
{
}

void App::SetWindow(GLFWwindow* Window) { window = Window; }
GLFWwindow* App::GetWindow() { return window; }

void App::SetGuiRenderer(std::unique_ptr<CRenderGui>&& GuiRenderer) { guiRenderer = std::move(GuiRenderer); }
const std::unique_ptr<CRenderGui>& App::GetGuiRenderer() { return guiRenderer; }
