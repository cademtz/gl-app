#include "font.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <array>

// https://jrgraphix.net/r/Unicode/
static const std::pair<int, int> font_ranges[] = {
    { 0x20, 0x7F } // Basic Latin
};

constexpr size_t font_ranges_count = sizeof(font_ranges) / sizeof(font_ranges[0]);

std::shared_ptr<CFont> CFont::LoadFromResource(const std::string& Name, int FontHeight)
{
    CResource::Load(Name);

    std::array<stbtt_pack_range, font_ranges_count> ranges = { 0 };
    uint32_t total_chars = 0;
    for (size_t i = 0; i < ranges.size(); ++i) {
        ranges[i].first_unicode_codepoint_in_range = font_ranges[i].first;
        ranges[i].num_chars = font_ranges[i].second;
        ranges[i].font_size = FontHeight;
        total_chars += ranges[i].num_chars;
    }
    
    uint32_t atlasRows = (uint32_t)sqrt(total_chars);
    
    stbtt_packedchar packed_chars[ranges.size()];

    uint32_t atlasW = FontHeight * , atlasH = FontHeight * atlasRows;
    std::vector<unsigned char> bitmap(atlasW * atlasH, 0);

    stbtt_pack_context pack;

    stbtt_PackBegin(&pack, &bitmap[0], atlasW, atlasH, 0, 1, nullptr);
    stbtt_PackSetOversampling(&pack, 0, 0);
    stbtt_PackFontRanges(&pack, (const unsigned char*)RawData->Data(), 0, &ranges[0], ranges.size());
    stbtt_PackEnd();
    stbtt_GetPackedQuad();
}

CFont::CFont() : CTextureStream(0, 0)
{
    
}