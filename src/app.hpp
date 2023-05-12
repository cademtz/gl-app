#pragma once
#include <platform.hpp>
#include <render/gui/rendergui.hpp>
#include <memory>

namespace App
{
    /**
     * @brief Called before running the app
     */
    void OnSetup();
    
    /**
     * @brief Called after running the app
     */
    void OnCleanup();
    /**
     * @brief Render to the current frame buffer
     */
    void Render();

    void SetGuiRenderer(std::unique_ptr<gui::RenderGui>&& Render);
    const std::unique_ptr<gui::RenderGui>& GetGuiRenderer();
};