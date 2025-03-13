#define STB_TRUETYPE_IMPLEMENTATION

#include "font.hpp"
#include <unordered_map>
#include <iostream>
#include <cmath>
#include <resources/resource.hpp>

std::optional<TrueType> TrueType::FromTrueType(std::shared_ptr<Resource> truetype) {
    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, truetype->UData(), 0))
        return std::nullopt;
    return TrueType(truetype, info);
}

uint32_t TrueType::FindGlyphId(uint32_t codepoint) const {
    return (uint32_t)stbtt_FindGlyphIndex(&m_info, (int)codepoint);
}

int32_t TrueType::GetGlyphKerning(uint32_t glyph1, uint32_t glyph2) const {
    return (int32_t)stbtt_GetGlyphKernAdvance(&m_info, (int)glyph1, (int)glyph2);
}

bool TrueType::GetGlyphMetrics(uint32_t glyph, FontGlyphMetrics *metrics) const
{
    int junk;
    if (!stbtt_GetGlyphBox(&m_info, glyph, &metrics->x0, &metrics->y0, &metrics->x1, &metrics->y1)) {
        //return false;
        *metrics = { 0 };
    }
    stbtt_GetGlyphHMetrics(&m_info, glyph, &metrics->next_x_offset, &junk);
    return true;
}

void TrueType::GetLineMetrics(FontLineMetrics* metrics) const {
    stbtt_GetFontVMetrics(&m_info, &metrics->line_y0, &metrics->line_y1, &metrics->gap);
}

bool TrueType::IsGlyphEmpty(uint32_t glyph) const {
    return stbtt_IsGlyphEmpty(&m_info, glyph);
}

bool TrueType::GetGlyphBitmapBox(
    uint32_t glyph,
    float scale_x, float scale_y,
    float shift_x, float shift_y,
    int32_t* x0, int32_t* y0,
    int32_t* x1, int32_t* y1
) const {
    stbtt_GetGlyphBitmapBoxSubpixel(&m_info, glyph, scale_x, scale_y, shift_x, shift_y, x0, y0, x1, y1);
    return true;
}

bool TrueType::MakeGlyphBitmap(
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

float TrueType::ScaleForPixelHeight(float height) const {
    FontLineMetrics metrics;
    GetLineMetrics(&metrics);
    return height / (metrics.line_y0 - metrics.line_y1);
}

const int32_t* FontCodepointMap::FindKerning(uint32_t first_glyph, uint32_t second_glyph) const {
    auto it = m_kerning_map.find(FontGlyphPair(first_glyph, second_glyph));
    if (it == m_kerning_map.cend())
        return nullptr;
    return &it->second;
}

void FontCodepointMap::AddCodepoint(const TrueType &truetype, codepoint_t codepoint) {
    FontGlyphInfo glyph = { 0 };
    glyph.id = truetype.FindGlyphId(codepoint);
    if (glyph.id == 0) {
        std::cout << "Invalid glyph" << std::endl;
        return;
    }
    
    truetype.GetGlyphMetrics(glyph.id, &glyph.metrics);
    if (truetype.IsGlyphEmpty(glyph.id)) {
        std::cout << "Huh, this valid glyph is actually empty!" << std::endl;
        //std::cout << "advance = " << glyph->metrics.next_x_offset * m_scale << std::endl;
    } else {
        
    }

    for (auto it = m_glyph_map.cbegin(); it != m_glyph_map.cend(); ++it) {
        uint32_t left = glyph.id, right = it->first;
        for (uint8_t i = 0; i < 2; ++i) {
            int32_t kerning = truetype.GetGlyphKerning(left, right);
            if (kerning != 0)
                m_kerning_map.insert({FontGlyphPair(left, right), kerning});
            left = it->first, right = glyph.id;
        }
    }
    
    m_glyph_map.insert({codepoint, std::move(glyph)});
}

void FontCodepointMap::AddRange(const TrueType& truetype, codepoint_t first_codepoint, codepoint_t last_codepoint) {
    for (codepoint_t i = first_codepoint; i <= last_codepoint; ++i) {
        AddCodepoint(truetype, i);
    }
}
