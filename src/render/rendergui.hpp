#pragma once
#include "render.hpp"
#include "texture.hpp"
#include "geometry.hpp"
#include "bake.hpp"
#include <glm/glm.hpp>
#include <font.hpp>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/**
 * @brief Render 2D geometry in screen-space pixel units.
 * Pixel units start from the top-left (0, 0) and end at the width and height of the current buffer.
 */
class CRenderGui : public CRender
{
public:
    CRenderGui(const std::string& DebugName) : CRender(DebugName) { }

    /**
     * @brief Set the geometry buffer to be rendered by @ref Render.
     * When `Geometry.texture == nullptr`, a default white texure is used
     */
    virtual void UploadGeometry(const Geometry2d& Geometry) = 0;

    float ScreenWidth() const { return m_screen_w; }
    float ScreenHeight() const { return m_screen_h; }
    void SetScreenSize(float Width, float Height) {
        m_screen_w = Width, m_screen_h = Height;
    }

    inline uint32_t AddFont(CFontBakeConfig&& config) {
        m_font_configs.emplace(m_next_font_id, config);
        return m_next_font_id++;
    }

    bool AppendCharacter(Geometry2d* geometry, glm::vec2 pos, uint32_t font_handle, uint32_t codepoint) const;

private:
    struct BakedFont;

    inline const CFontBakeConfig* GetFontConfig(uint32_t font) const {
        auto it = m_font_configs.find(font);
        return it != m_font_configs.cend() ? &it->second : nullptr;
    }

    inline const BakedFont* GetBakedFont(uint32_t font) const {
        auto it = m_baked_fonts.find(font);
        return it != m_baked_fonts.cend() ? &it->second : nullptr;
    }

    inline BakedFont* GetBakedFont(uint32_t font) {
        return const_cast<BakedFont*>(((const CRenderGui*)this)->GetBakedFont(font));
    }

    /**
     * @brief Bake everything into an atlas, swapping the current atlas (if any)
     */
    void CreateAtlas();

    float m_screen_w = 0;
    float m_screen_h = 0;

    struct BakedFont
    {
        std::unordered_map<uint32_t, CFontBakedGlyph> m_glyphs;
        std::unordered_map<uint32_t, uint32_t> m_codepoints;
    };
    
    std::unordered_map<uint32_t, BakedFont> m_baked_fonts;
    std::unordered_map<uint32_t, CFontBakeConfig> m_font_configs;
    uint32_t m_next_font_id = 1;
    std::shared_ptr<CTexture> m_atlas_texture;
    

    std::unordered_map<uint32_t, BakedFont> m_fonts;
};