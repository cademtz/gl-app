#pragma once
#include "forward.hpp"
#include "render2d_list.hpp"
#include "render2d_draw.hpp"

namespace Render2d {
    inline float m_screen_w = 0;
    inline float m_screen_h = 0;
    /// @brief `nullptr` will render to the screen
    inline TexturePtr render_target = nullptr;
    static const TextureFormat OUTPUT_FORMAT = TextureFormat::RGBA_8_32;

    bool Setup();
    void Cleanup();

    void UploadDrawData(const DrawList& list);
    void Render();
}