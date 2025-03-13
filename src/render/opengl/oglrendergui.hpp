#pragma once
#include <render/gui/forward.hpp>
#include <render/forward.hpp>
#include "opengl.hpp"

namespace gui { class DrawList; }

namespace OglRenderGui {
    inline float m_screen_w = 0;
    inline float m_screen_h = 0;
    /// @brief `nullptr` will render to the screen
    inline TexturePtr render_target = nullptr;
    static const TextureFormat OUTPUT_FORMAT = TextureFormat::RGBA_8_32;

    bool Setup();
    void Cleanup();

    void UploadDrawData(const gui::DrawList& list);
    void Render();
};