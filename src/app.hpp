#pragma once
#include <platform.hpp>
#include <render/rendergui.hpp>
#include <memory>

namespace App
{
    void OnSetup();
    void OnCleanup();

    /**
     * @brief Perform all the app's logic in one call, without waiting.
     *  Handle pending inputs, render the next frame, and more.
     */
    void Loop();

    void SetGuiRenderer(std::unique_ptr<CRenderGui>&& Render);
    const std::unique_ptr<CRenderGui>& GetGuiRenderer();
};