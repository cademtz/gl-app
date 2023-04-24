#pragma once

#include "resources/resource.hpp"
#include <stb_truetype.h>
#include <unordered_map>
#include <fnv1a.hpp>
#include <optional>
#include <cstdint>
#include <string>
#include <vector>

// TODO: Make an iterable list of all known ranges.
// This way, if a user types a previously-unloaded codepoint then
// the app can load the entire block it belongs in
/**
 * @brief Get unicode ranges
 */

typedef uint32_t codepoint_t;

struct UnicodeRange {
    codepoint_t begin, end;
};

namespace UnicodeBlocks {
    const static UnicodeRange
        BASIC_LATIN = { 0x20, 0x7F },
        LATIN_EXTENDED_A = { 0x100, 0x17F },
        LATIN_EXTENDED_B = { 0x180, 0x24F };
}

/**
 * @brief Parameters to load and rasterize a font
 */
struct CFontBakeConfig
{   
    CFontBakeConfig(std::string&& url, float height, UnicodeRange range = UnicodeBlocks::LATIN_EXTENDED_A)
    : url(url), height(height) {
        ranges.push_back(range);
    }

    /**
     * @brief Initialize the config with multiple unicode ranges
     * @param begin 
     */
    template <class T>
    CFontBakeConfig(std::string&& url, float height, T begin, T end)
    : url(url), height(height) {
        ranges.insert(ranges.cbegin(), begin, end);
    }

    /**
     * @return Location of the font file
     */
    std::string url;
    /**
     * @return Desired height, in pixels
     */
    float height;

    std::vector<UnicodeRange> ranges;
};

/**
 * @brief A font's line size and placement.
 * The Y axis is relative to the baseline, such that +y is above and -y is below.
 */
struct CFontLineMetrics {
    /**
     * @brief Top of a line
     */
    int32_t line_y0;
    /**
     * @brief Bottom of a line (typically negative)
     */
    int32_t line_y1;
    /**
     * @brief Additional vertical space between the bottom and top of two lines
     */
    int32_t gap;
};

/**
 * @brief A glyph's size and placement.
 * The X axis is relative to the cursor, such that +x extends right and -x extends left.
 * The Y axis is relative to the baseline, such that +y is above and -y is below.
 */
struct CFontGlyphMetrics
{
    /**
     * @brief Bounding box of all visible parts of the glyph.
     */
    int32_t x0, y0, x1, y1;
    /**
     * @brief Horizontal offset to move the cursor to its next position after this glyph
     */
    int32_t next_x_offset;
};

/**
 * @brief Pixel measurements to place and render a rasterized glyph
 */
struct CFontBakedGlyph {
    /**
     * @brief Coordinates of the baked glyph inside the bitmap.
     * These values may be zero for missing glyphs or whitespace codepoints
     */
    uint16_t bmp_x0, bmp_y0, bmp_x1, bmp_y1;
    /**
     * @brief Horizontal offset of the next glyph, relative to the cursor
     */
    float next_x_offset;
    /**
     * @brief Placement of the bitmap box's top-left corner, relative to the cursor
     */
    float place_x_offset, place_y_offset;
};

/**
 * @brief Extract information from a truetype font.
 * This is essentially a wrapper for stb_truetype.
 */
class CTrueType
{
public:
    using Ptr = std::shared_ptr<CTrueType>;
    
    static std::optional<CTrueType> FromTrueType(std::shared_ptr<CResource> truetype);

    /**
     * @return ID of the corresponding glyph, or `0` if not found
     */
    uint32_t FindGlyphId(uint32_t codepoint) const;

    /**
     * @return Additional horizontal space between two glyphs
     */
    int32_t GetGlyphKerning(uint32_t glyph1, uint32_t glyph2) const;

    /**
     * @brief Get the metrics of a glyph
     * @param glyph ID of the glyph
     * @param out_metrics Will be updated with the glyph's metrics
     * @return `false` if the glyph ID was not found
     */
    bool GetGlyphMetrics(uint32_t glyph, CFontGlyphMetrics* out_metrics) const;

    /**
     * @brief Get the vertical metrics of a line
     * @param metrics Will be updated with line metrics
     */
    void GetLineMetrics(CFontLineMetrics* out_metrics) const;

    bool IsGlyphEmpty(uint32_t glyph) const;

    /**
     * @brief Get the size of a glyph's bitmap
     * @param shift_x Sub-pixel shift from 0-1
     * @param shift_y Sub-pixel shift from 0-1
     * @return `true` if the glyph was found and values were returned
     */
    bool GetGlyphBitmapBox(
        uint32_t glyph,
        float scale_x, float scale_y,
        float shift_x, float shift_y,
        int32_t* out_x0, int32_t* out_y0,
        int32_t* out_x1, int32_t* out_y1
    ) const;

    /**
     * @brief Render a glyph to a single-channel 8-bit bitmap
     * You can calculate the necessary width and height with @ref GetGlyphBitmapBox
     * @param shift_x Sub-pixel shift from 0-1
     * @param shift_y Sub-pixel shift from 0-1
     * @param buffer Location in an existing bitmap to write font data
     * @param clamp_w Clamps the width that can be rendered to
     * @param clamp_h Clamps the height that can be rendered to
     * @param row_stride Number of bytes in a row
     * @see GetGlyphBitmapBox
     */
    bool MakeGlyphBitmap(
        uint32_t glyph,
        float scale_x, float scale_y,
        float shift_x, float shift_y,
        uint8_t* buffer,
        size_t clamp_w, size_t clamp_h,
        uint32_t row_stride
    ) const;

    float ScaleForPixelHeight(float height) const;

    void GetTextSize(const wchar_t* text, float scale_factor, uint32_t* out_width, uint32_t* out_height);

private:
    CTrueType(std::shared_ptr<CResource> truetype, stbtt_fontinfo info)
        : m_truetype(truetype), m_info(info) {}

    const std::shared_ptr<CResource> m_truetype;
    const stbtt_fontinfo m_info;
};

struct CFontGlyphInfo
{
    /**
     * @brief Font-specific glyph ID
     */
    uint32_t id;
    CFontGlyphMetrics metrics;
};

/**
 * @brief Hash two glyphs
 */
class CFontGlyphPair : public std::pair<uint32_t, uint32_t>
{
public:
    // Forward constructor
    using std::pair<uint32_t, uint32_t>::pair;

    uint32_t Hash() const {
        uint32_t hash = fnv1a::Hash_32(first);
        hash = fnv1a::Hash_32(second, hash);
        return hash;
    }
};

/**
 * @brief Map a font's codepoints to their glyphs, and two glyphs to the kerning offset between them
 */
class CFontCodePointMap
{
public:
    CFontCodePointMap() {}

    /**
     * @brief Calculate text size from the first pixel of the first letter to the last pixel of the last letter
     */
    void GetTextSize(const wchar_t* text, uint32_t* out_width, uint32_t* out_height) const;

    const CFontGlyphInfo* FindGlyph(uint32_t codepoint) const {
        auto iter = m_glyph_map.find(codepoint);
        if (iter == m_glyph_map.cend())
            return nullptr;
        return &iter->second;
    }

    void AddCodepoint(const CTrueType& truetype, uint32_t codepoint);
    void AddRange(const CTrueType& truetype, uint32_t first_codepoint, uint32_t last_codepoint);

private:
    struct GlyphPairHasher {
        uint32_t operator()(const CFontGlyphPair& pair) const {
            return pair.Hash();
        }
    };

    std::unordered_map<CFontGlyphPair, int32_t, GlyphPairHasher> m_kerning_map;
    std::unordered_map<uint32_t, CFontGlyphInfo> m_glyph_map;
};