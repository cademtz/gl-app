#pragma once

#include <stb_truetype.h>
#include <unordered_map>
#include <fnv1a.hpp>
#include <optional>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

// TODO: Make an iterable list of all known ranges.
// This way, if a user types a previously-unloaded codepoint then
// the app can load the entire block it belongs in

typedef uint32_t codepoint_t;
class Resource;

struct UnicodeRange {
    codepoint_t begin, end;
    
    bool Contains(codepoint_t cp) const { return cp >= begin && cp <= end; }
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
struct FontBakeConfig {
    FontBakeConfig(std::string&& url, float height_px, uint8_t oversample = 1, UnicodeRange range = UnicodeBlocks::BASIC_LATIN)
    : url(std::move(url)), height_px(height_px), oversample(oversample) {
        ranges.push_back(range);
    }

    /**
     * @brief Initialize the config with multiple unicode ranges
     * @param begin Beginning UnicodeRange iterator
     * @param end Ending UnicodeRange iterator
     */
    template <class T>
    FontBakeConfig(std::string&& url, float height_px, uint8_t oversample, T begin, T end)
    : url(std::move(url)), height_px(height_px), oversample(oversample) {
        ranges.insert(ranges.cbegin(), begin, end);
    }

    /** Location of the font file */
    std::string url;
    /** Desired height, in pixels */
    float height_px;
    /** Size multiplier for more precise downscaling */
    uint8_t oversample;

    std::vector<UnicodeRange> ranges;
};

/**
 * @brief A font's line size and placement.
 * The Y axis is relative to the baseline, such that +y is above and -y is below.
 */
struct FontLineMetrics {
    /** Top of a line */
    int32_t line_y0;
    /** Bottom of a line (typically negative) */
    int32_t line_y1;
    /** Additional vertical space between the bottom and top of two lines */
    int32_t gap;

    float GetHeight(float scale = 1.f) const { return (line_y1 - line_y0) * scale; }
};

/**
 * @brief A glyph's size and placement.
 * The X axis is relative to the cursor, such that +x extends right and -x extends left.
 * The Y axis is relative to the baseline, such that +y is above and -y is below.
 */
struct FontGlyphMetrics {
    /**
     * @brief Bounding box of all visible parts of the glyph.
     * The first coordinate is top-left
     */
    int32_t x0, y0, x1, y1;
    /** Horizontal offset to move the cursor to its next position after this glyph */
    int32_t next_x_offset;
    
    float GetWidth(float scale = 1.f) const { return (x1 - x0) * scale; }
    float GetHeight(float scale = 1.f) const { return (y1 - y0) * scale; }
    bool IsEmpty() const { return x0 == x1 || y0 == y1; }
};

/**
 * @brief Extract information from a truetype font.
 * This is essentially a wrapper for `stb_truetype.h`.
 */
class TrueType
{
public:
    using Ptr = std::shared_ptr<TrueType>;
    
    static std::optional<TrueType> FromTrueType(std::shared_ptr<Resource> truetype);

    /** @return ID of the corresponding glyph, or `0` if not found */
    uint32_t FindGlyphId(uint32_t codepoint) const;

    /** @return Additional horizontal space between two glyphs */
    int32_t GetGlyphKerning(uint32_t glyph1, uint32_t glyph2) const;

    /**
     * @brief Get the metrics of a glyph
     * @param glyph ID of the glyph
     * @param out_metrics Will be updated with the glyph's metrics
     * @return `false` if the glyph ID was not found
     */
    bool GetGlyphMetrics(uint32_t glyph, FontGlyphMetrics* out_metrics) const;

    /**
     * @brief Get the vertical metrics of a line
     * @param metrics Will be updated with line metrics
     */
    void GetLineMetrics(FontLineMetrics* out_metrics) const;

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
    TrueType(std::shared_ptr<Resource> truetype, stbtt_fontinfo info)
        : m_truetype(truetype), m_info(info) {}

    const std::shared_ptr<Resource> m_truetype;
    const stbtt_fontinfo m_info;
};

struct FontGlyphInfo {
    /**
     * @brief Font-specific glyph ID
     */
    uint32_t id;
    FontGlyphMetrics metrics;
};

/**
 * @brief Hash two glyphs
 */
class FontGlyphPair : public std::pair<uint32_t, uint32_t> {
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
class FontCodepointMap {
public:
    FontCodepointMap() {}

    /**
     * @brief Calculate text size from the first pixel of the first letter to the last pixel of the last letter
     */
    void GetTextSize(const wchar_t* text, uint32_t* out_width, uint32_t* out_height) const;

    /**
     * @return @ref FontGlyphInfo, or `nullptr` if the glyph is not mapped
     */
    const FontGlyphInfo* FindGlyph(codepoint_t codepoint) const {
        auto iter = m_glyph_map.find(codepoint);
        if (iter == m_glyph_map.cend())
            return nullptr;
        return &iter->second;
    }

    /**
     * @return The kerning between two glyphs, or `nullptr` if none was found
     */
    const int32_t* FindKerning(uint32_t first_glyph, uint32_t second_glyph) const;

    void AddCodepoint(const TrueType& truetype, codepoint_t codepoint);
    void AddRange(const TrueType& truetype, codepoint_t first_codepoint, codepoint_t last_codepoint);

private:
    struct GlyphPairHasher {
        uint32_t operator()(const FontGlyphPair& pair) const {
            return pair.Hash();
        }
    };

    std::unordered_map<FontGlyphPair, int32_t, GlyphPairHasher> m_kerning_map;
    std::unordered_map<uint32_t, FontGlyphInfo> m_glyph_map;
};