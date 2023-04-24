#define STB_TRUETYPE_IMPLEMENTATION

#include "font.hpp"
#include <unordered_map>
#include <iostream>
#include <cmath>

std::optional<CTrueType> CTrueType::FromTrueType(std::shared_ptr<CResource> truetype) {
    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, truetype->UData(), 0))
        return std::nullopt;
    return CTrueType(truetype, info);
}

uint32_t CTrueType::FindGlyphId(uint32_t codepoint) const {
    return (uint32_t)stbtt_FindGlyphIndex(&m_info, (int)codepoint);
}

int32_t CTrueType::GetGlyphKerning(uint32_t glyph1, uint32_t glyph2) const {
    return (int32_t)stbtt_GetGlyphKernAdvance(&m_info, (int)glyph1, (int)glyph2);
}

bool CTrueType::GetGlyphMetrics(uint32_t glyph, CFontGlyphMetrics *metrics) const
{
    int junk;
    if (!stbtt_GetGlyphBox(&m_info, glyph, &metrics->x0, &metrics->y0, &metrics->x1, &metrics->y1))
        return false;
    stbtt_GetGlyphHMetrics(&m_info, glyph, &metrics->next_x_offset, &junk);
    return true;
}

void CTrueType::GetLineMetrics(CFontLineMetrics* metrics) const {
    stbtt_GetFontVMetrics(&m_info, &metrics->line_y0, &metrics->line_y1, &metrics->gap);
}

bool CTrueType::IsGlyphEmpty(uint32_t glyph) const {
    return stbtt_IsGlyphEmpty(&m_info, glyph);
}

bool CTrueType::GetGlyphBitmapBox(
    uint32_t glyph,
    float scale_x, float scale_y,
    float shift_x, float shift_y,
    int32_t* x0, int32_t* y0,
    int32_t* x1, int32_t* y1
) const {
    stbtt_GetGlyphBitmapBoxSubpixel(&m_info, glyph, scale_x, scale_y, shift_x, shift_y, x0, y0, x1, y1);
    return true;
}

bool CTrueType::MakeGlyphBitmap(
    uint32_t glyph,
    float scale_x, float scale_y,
    float shift_x, float shift_y,
    uint8_t* buffer,
    size_t clamp_w, size_t clamp_h,
    uint32_t row_stride
) const {
    stbtt_MakeGlyphBitmapSubpixel(&m_info, buffer, clamp_w, clamp_h, row_stride, scale_x, scale_y, shift_x, shift_y, glyph);
    return true;
}

float CTrueType::ScaleForPixelHeight(float height) const {
    CFontLineMetrics metrics;
    GetLineMetrics(&metrics);
    return height / (metrics.line_y0 - metrics.line_y1);
}

void CFontCodePointMap::AddCodepoint(const CTrueType &truetype, uint32_t codepoint)
{
    CFontGlyphInfo glyph;
    glyph.id = truetype.FindGlyphId(codepoint);
    if (glyph.id == 0)
    {
        std::cout << "Invalid glyph" << std::endl;
        return;
    }
    if (truetype.IsGlyphEmpty(glyph.id))
        std::cout << "Huh, this valid glyph is actually empty!" << std::endl;
    
    truetype.GetGlyphMetrics(glyph.id, &glyph.metrics);
    for (auto it = m_glyph_map.cbegin(); it != m_glyph_map.cend(); ++it) {
        int32_t kerning = truetype.GetGlyphKerning(glyph.id, it->first);
        if (kerning != 0)
            m_kerning_map.insert({CFontGlyphPair(glyph.id, it->first), kerning});
    }

    m_glyph_map.insert({glyph.id, glyph});
}

void CFontCodePointMap::AddRange(const CTrueType& truetype, uint32_t first_codepoint, uint32_t last_codepoint)
{
    for (size_t i = first_codepoint; i <= last_codepoint; ++i) {
        AddCodepoint(truetype, i);
    }
}
