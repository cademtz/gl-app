#pragma once
#include <platform.hpp>
#include <render/rendergui.hpp>
#include <memory>

namespace App
{
    void OnStartup();
    void OnClose();
    void Loop();
    void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    void CursorCallback(GLFWwindow* window, double xpos, double ypos);

    void SetWindow(GLFWwindow* Window);
    GLFWwindow* GetWindow();

    void SetGuiRenderer(std::unique_ptr<CRenderGui>&& Render);
    const std::unique_ptr<CRenderGui>& GetGuiRenderer();
};