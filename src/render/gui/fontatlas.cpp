#include "fontatlas.hpp"
#include "font.hpp"
#include <render/texture.hpp>
#include <render/bake.hpp>
#include <cmath>

#include <cassert>
#include <iostream>

namespace gui {

FontAtlas::FontAtlas(const TrueType& tt, const FontBakeConfig& cfg)
: m_scale(tt.ScaleForPixelHeight(cfg.height_px)), m_oversample(cfg.oversample) {
    ClientTexturePtr glyph_bmp = nullptr;
    std::unordered_map<uint32_t, uint32_t> glyph_to_rect;
    RectPacker rectpack;

    tt.GetLineMetrics(&m_line);

    for (const UnicodeRange& range : cfg.ranges) {
        m_codepoint_map.AddRange(tt, range.begin, range.end);

        for (codepoint_t cp = range.begin; cp <= range.end; ++cp) {
            const FontGlyphInfo* glyph = m_codepoint_map.FindGlyph(cp);
            if (glyph == nullptr)
                continue;

            auto it = m_glyph_map.find(glyph->id);
            if (it != m_glyph_map.end())
                continue; // Glyph is already in the map
            
            const FontGlyphMetrics& metrics = glyph->metrics;

            uint16_t bmp_w = std::ceil(metrics.GetWidth(m_scale));
            uint16_t bmp_h = std::ceil(metrics.GetHeight(m_scale));
            if (bmp_w == 0 || bmp_h == 0)
                glyph_to_rect[glyph->id] = ~(uint32_t)0;
            bmp_w += 1; // Extend width by 1 pixel because it otherwise *still* gets cut off in a few cases
            glyph_to_rect[glyph->id] = rectpack.AddRect(bmp_w, bmp_h);
        }
    }

    rectpack.Pack();
    assert(rectpack.IsPacked());
    
    uint32_t atlas_w, atlas_h;
    rectpack.GetPackedSize(&atlas_w, &atlas_h);
    
    auto atlas_tex = ClientTexture::Create(TextureInfo(TextureFormat::RGBA_8_32, atlas_w, atlas_h));

    for (auto it = glyph_to_rect.begin(); it != glyph_to_rect.end(); ++it) {
        uint32_t glyph_id = it->first;
        uint32_t rect_id = it->second;
        
        GlyphRect box = {0};
        if (rect_id != ~(uint32_t)0) {
            RectPacker::Rect packed_rect;
            rectpack.GetRect(rect_id, &packed_rect);
            box = {
                (uint16_t)packed_rect.x, (uint16_t)packed_rect.y,
                (uint16_t)packed_rect.w, (uint16_t)packed_rect.h
            };
            ClientTexturePtr glyph_rgba = ClientTexture::Create(TextureInfo(TextureFormat::RGBA_8_32, box.w, box.h));

            {
                GlyphRect ss_rect = box;
                ss_rect.x *= m_oversample, ss_rect.y *= m_oversample, ss_rect.w *= m_oversample, ss_rect.h *= m_oversample;

                // Rasterize
                if (!glyph_bmp || glyph_bmp->GetInfo().width < ss_rect.w || glyph_bmp->GetInfo().height < ss_rect.h)
                    glyph_bmp = ClientTexture::Create(TextureInfo(TextureFormat::A_8_8, ss_rect.w, ss_rect.h));

                bool ok = tt.MakeGlyphBitmap(
                    glyph_id,
                    m_scale * m_oversample, m_scale * m_oversample,
                    0, 0,
                    glyph_bmp->GetData(),
                    ss_rect.w, ss_rect.h,
                    glyph_bmp->GetInfo().GetRowStride()
                );
                assert(ok && "Failed to render glyph");

                for (uint32_t y = 0; y < box.h; ++y) {
                    for (uint32_t x = 0; x < box.w; ++x) {
                        uint8_t* output = glyph_rgba->GetPixel(x, y);
                        
                        float sum = 0;
                        for (uint8_t y_s = 0; y_s < m_oversample; ++y_s) {
                            for (uint8_t x_s = 0; x_s < m_oversample; ++x_s) {
                                const uint8_t* sample = glyph_bmp->GetPixel(x * m_oversample + x_s, y * m_oversample + y_s);
                                sum += *sample;
                            }
                        }
                        float avg = sum / (m_oversample * m_oversample);
                        output[0] = output[1] = output[2] = 0xFF;
                        output[3] = (uint8_t)avg;
                    }
                }
            }

            // Write to atlas

            atlas_tex->Write(glyph_rgba, box.x, box.y, box.w, box.h);
        }

        m_glyph_map[glyph_id] = box;
    }

    m_atlas_tex = Texture::Create(atlas_tex);
}

bool FontAtlas::GetGlyphTextureRect(uint32_t glyph_id, GlyphRect* out_rect) const {
    auto it = m_glyph_map.find(glyph_id);
    if (it == m_glyph_map.cend())
        return false;
    *out_rect = it->second;
    return true;
}

}
