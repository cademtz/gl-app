#include <font.hpp>
#include <memory>
#include <unordered_map>

class Texture;

namespace gui {

/**
 * @brief A font's codepoints pre-rendered into one large texture.
 */
class FontAtlas {
public:
    using Ptr = std::shared_ptr<FontAtlas>;
    using ConstPtr = std::shared_ptr<const FontAtlas>;
    /**
     * @brief Glyph texture coordinates in pixels.
     * Coordinates start at the top-left and end at the bottom-right.
     */
    struct GlyphRect {
        uint16_t x, y, w, h;
    };

    /**
     * @brief Construct the atlas with an already-loaded font
     */
    FontAtlas(const TrueType& tt, const FontBakeConfig& cfg);

    /**
     * @return Scale factor to convert font units to pixels
     */
    float GetScale() const { return m_scale; }
    uint8_t GetOversample() const { return m_oversample; }
    
    std::shared_ptr<Texture> GetTexture() const { return m_atlas_tex; }

    /**
     * @brief Get a glyph's texture rect.
     * Whitespace glyphs have a rect with zero width and height.
     * @param out_rect The @ref GlyphRect to be written
     * @return `true` if the glyph was found and `out_rect` was written
     */
    bool GetGlyphTextureRect(uint32_t glyph_id, GlyphRect* out_rect) const;

    const FontCodepointMap& GetCodepointMap() const { return m_codepoint_map; }
    const FontLineMetrics& GetLineMetrics() const { return m_line; }

private:
    const float m_scale;
    const uint8_t m_oversample = 1;
    std::shared_ptr<Texture> m_atlas_tex;

    std::unordered_map<uint32_t, GlyphRect> m_glyph_map;
    FontCodepointMap m_codepoint_map;
    FontLineMetrics m_line;
};

}