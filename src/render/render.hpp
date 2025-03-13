#pragma once
#include "texture.hpp"

/**
 * @brief The interface for everything that will output pixels to a screen.
 * Use @ref SetTarget to render to screen or texture.
 */
class CRender
{
public:
    virtual ~CRender() { }
    /** Render to the active target */
    virtual void Render() = 0;
    /** Supported output/target format. Currently, it's always RGBA. */
    virtual TextureFormat GetOutputFormat() = 0;
    /** @return The texture being rendered to, or `nullptr` for the screen */
    TexturePtr GetTarget() { return m_target; }
    /**
     * @brief Set the screen or texture to be rendered to
     * @param tex The texture, or `nullptr` for the screen
     */
    void SetTarget(TexturePtr tex = nullptr) {
        m_target = tex;
        SetTargetInternal();
    }

protected:
    /** Called immediately after @ref SetTarget. Might be useful. */
    virtual void SetTargetInternal() {}

private:
    TexturePtr m_target = nullptr;
};