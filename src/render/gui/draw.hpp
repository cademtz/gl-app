#pragma once
#include <cmath> // NAN
#include <vector>
#include <memory>
#include <font.hpp>
#include <string_view>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "drawlist.hpp"
#include "fontmanager.hpp"

namespace gui {

/**
 * @brief Draw with primitive geometry and textures in traditional screen coordinates.
 * Vertices at (0, 0) will appear at the top-left of the screen.
 * Texture coordinates at (0, 0) will use the top-left of the active texture.
 */
class Draw {
public:
    Draw();

    /** Clear the draw list and any previously assigned color, clip rect, etc. */
    void Clear();
    DrawList& GetDrawList() { return m_drawlist; }
    const DrawList& GetDrawList() const { return m_drawlist; }
    void SetColor(const glm::vec4& rgba);
    void SetColor(float r, float g, float b, float a = 1.f) { SetColor(glm::vec4(r,g,b,a)); }
    void ResetColor() { m_rgba = glm::vec4(1); }
    /**
     * @brief Set a new clip rect, wich is further clipped within the bounds of the previous rect.
     * @param new_clip Rectangle in `{ x, y, w, h }` format.
     *  The default value will not clip any geometry.
     */
    void PushClip(glm::vec4 new_clip = NO_CLIP);
    void PopClip();
    /** Draw a single unicode character*/
    void Codepoint(FontHandle font, codepoint_t codepoint, glm::vec2 top_left);
    /** Draw unicode text */
    void TextUnicode(FontHandle font, glm::vec2 top_left, std::u32string_view text);
    /** Draw ascii text */
    void TextAscii(FontHandle font, glm::vec2 top_left, std::string_view text);
    /** Draw a font's atlas for debugging purposes. */
    void DebugFontAtlas(FontHandle font, glm::vec2 top_left, glm::vec2 size = glm::vec2(NAN));
    void Rect(glm::vec2 top_left, glm::vec2 size);
    void Rect(float x, float y, float w, float h) { Rect(glm::vec2(x, y), glm::vec2(w, h)); }
    /**
     * @param texture Current texture (can be `nullptr`)
     * @param size Rectangle size. The default value will use the texture's size.
     *  If `texture` is `nullptr`, nothing will be drawn.
     */
    void TextureRect(Texture::Ptr texture, glm::vec2 top_left, glm::vec2 size = glm::vec2(NAN));
    void Ellipse(uint32_t num_points, glm::vec2 top_left, glm::vec2 size);
    void Ellipse(uint32_t num_points, float x, float y, float w, float h) { Ellipse(num_points, glm::vec2(x, y), glm::vec2(w, h)); }
    /**
     * @param texture Current texture (can be `nullptr`)
     * @param size Ellipse size. The default value will use the texture's size.
     *  If `texture` is `nullptr`, nothing will be drawn.
     */
    void TextureEllipse(Texture::Ptr texture, uint32_t num_points, glm::vec2 top_left, glm::vec2 size = glm::vec2(NAN));
    
private:
    void TextInternal(FontHandle font, glm::vec2 top_left, const void* begin, const void* end, uint8_t stride);
    /** Internal utility to add rectangle geometry */
    void RectUv(glm::vec2 xy, glm::vec2 size, glm::vec2 uv, glm::vec2 uv_wh);
    /** Internal utility to add ellipse geometry */
    void EllipseUv(uint32_t num_points, glm::vec2 xy, glm::vec2 size, glm::vec2 uv, glm::vec2 uv_wh);

    /** @return `value` if `value` is not NaN. Otherwise, `default_value` */
    static glm::vec2 VecOrDefault(glm::vec2 value, glm::vec2 default_value); 
    void SetTexture(Texture::Ptr texture);
    /** Call when the current params like texture or clipping are changing */
    void OnDrawParamsChanged();
    /**
     * @brief Add a new draw call that draws the last number of indices, `num_indices`, with the current clipping rect and texture.
     * In most cases, it's appended to a previous draw call with the same parameters.
     * @param num_indices The number of new vertices added. Must be a multiple of 3.
     */
    void AddDrawCall(uint32_t num_indices);
    DrawCall* GetDrawCall();

    glm::vec4 m_rgba = glm::vec4(1);
    DrawList m_drawlist;
    /**
     * @brief Assigned to the latest existing draw call if it has the same parameters.
     * Do not access directly. Use @ref GetDrawCall instead.
     * @see GetDrawCall
     */
    DrawCall* m_drawcall = nullptr;
    Texture::Ptr m_texture = nullptr;
    glm::vec4 m_clip = NO_CLIP;
    std::vector<glm::vec4> m_clip_stack;
};

}