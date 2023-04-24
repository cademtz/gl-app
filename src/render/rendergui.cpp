#include "rendergui.hpp"
#include "font.hpp"
#include "resources/resource.hpp"
#include "bake.hpp"

// Debugging
#include <cstdint>
#include <iostream>
#include <cassert>
#include <memory>

void CRenderGui::CreateAtlas()
{
    CRectPacker packer;
    // Font ID to truetype
    std::unordered_map<uint32_t, CTrueType> map_truetype;

    // Pack all character rectangles
    for (const auto& node : m_font_configs)
    {
        uint32_t font_id = node.first;
        const CFontBakeConfig& font_cfg = node.second;
        auto res = CResource::LoadSynchronous(font_cfg.url);
        if (!res)
        {
            std::cout << "Failed to load font from " << font_cfg.url << std::endl;
            continue;
        }

        std::optional<CTrueType> tt_loaded = CTrueType::FromTrueType(res);
        if (!tt_loaded)
        {
            std::cout << "Couldn't load truetype from data in " << font_cfg.url << std::endl;
            continue;
        }

        CTrueType& tt = map_truetype.emplace(std::make_pair(font_id, std::move(*tt_loaded))).first->second;
        float scale = tt.ScaleForPixelHeight(font_cfg.height);

        for (const UnicodeRange& range : font_cfg.ranges) {
            for (codepoint_t cp = range.begin; cp < range.end; ++cp)
            {
                uint32_t glyph = tt.FindGlyphId(cp);
                int32_t x0, y0, x1, y1;

                if (glyph == 0)
                    continue;
                
                tt.GetGlyphBitmapBox(
                    glyph,
                    scale, scale,
                    0, 0,
                    &x0, &y0, &x1, &y1
                );

                uint32_t width = x1 - x0, height = y0 - y1;
                packer.AddRect(width, height);
            }
        }
    }
    
    if (!packer.Pack())
        assert(0 && "Epic fail");
    
    uint32_t packed_w, packed_h;
    packer.GetPackedSize(&packed_w, &packed_h);

    const uint32_t px_stride = 4; // RGBA
    const uint32_t row_stride = px_stride * packed_w;
    auto buffer = std::make_unique<uint8_t[]>(packed_w * packed_h * px_stride);

    // Render all characters' rectangles
    for (const auto& node : m_font_configs)
    {
        uint32_t font_id = node.first;
        const CFontBakeConfig& font_cfg = node.second;
        const CTrueType& tt = map_truetype.find(font_id)->second;
        float scale = tt.ScaleForPixelHeight(font_cfg.height);
        for (const UnicodeRange& range : font_cfg.ranges) {
            for (codepoint_t cp = range.begin; cp < range.end; ++cp)
            {
                uint32_t glyph = tt.FindGlyphId(cp);

                if (glyph == 0)
                    continue;
                
                tt.MakeGlyphBitmap(
                    glyph,
                    scale, scale,
                    0, 0, buffer.get(), -1, -1, row_stride
                );
            }
        }
    }
}

bool CRenderGui::AppendCharacter(Geometry2d *geometry, glm::vec2 pos, uint32_t font_handle, uint32_t codepoint) const
{
    auto font_iter = m_fonts.find(font_handle);
    if (font_iter == m_fonts.cend())
        return false;
    
    const BakedFont& font = font_iter->second;
    auto glyph_id_iter = font.m_codepoints.find(codepoint);
    if (glyph_id_iter == font.m_codepoints.cend())
        return false;
    
    uint32_t glyph_id = glyph_id_iter->second;
    auto glyph_iter = font.m_glyphs.find(glyph_id);
    if (glyph_iter == font.m_glyphs.cend())
        return false;
    
    const CFontBakedGlyph& glyph = glyph_iter->second;
    uint32_t start_index = geometry->vertices.size();
    uint32_t width = glyph.bmp_x1 - glyph.bmp_x0;
    uint32_t height = glyph.bmp_y1 - glyph.bmp_y0;

    std::array<std::array<bool, 2>, 4> verts = {{
        0, 0,
        0, 1,
        1, 1,
        1, 0
    }};

    std::array<uint32_t, 6> indices = {
        0, 1, 2,
        2, 3, 0 
    };

    for (auto& v : verts) {
        geometry->vertices.push_back({
            pos.x + width * v[0], pos.y + height * v[0],
            (float)glyph.bmp_x0 + width * v[0], (float)glyph.bmp_y0 + height * v[1],
            1, 1, 1, 1
        });
    }

    for (auto index : indices)
        geometry->indices.push_back(index + start_index);
    
    return true;
}