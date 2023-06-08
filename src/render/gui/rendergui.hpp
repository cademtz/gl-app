#pragma once
#include <memory>
#include <render/render.hpp>

namespace gui {

class DrawList;

/**
 * @brief Render 2D geometry in screen-space pixel units.
 * Pixel units start from the top-left (0, 0) and end at the width and height of the current buffer.
 */
class RenderGui : public CRender
{
public:
    RenderGui(const std::string& debug_name) : CRender(debug_name) { }

    static std::shared_ptr<RenderGui> GetInstance();

    /**
     * @brief Set the data to be rendered by @ref Render.
     * For any `nullptr` texture, a default white texure will be used.
     */
    virtual void UploadDrawData(const DrawList& list) = 0;

    float ScreenWidth() const { return m_screen_w; }
    float ScreenHeight() const { return m_screen_h; }
    void SetScreenSize(float Width, float Height) {
        m_screen_w = Width, m_screen_h = Height;
    }

private:
    float m_screen_w = 0;
    float m_screen_h = 0;
};

}