#pragma once
#include "forward.hpp"
#include "font/forward.hpp"
#include "render2d_list.hpp"
#include "opengl/oglshader.hpp"
#include <cmath> // NAN
#include <vector>
#include <memory>
#include <string_view>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

namespace Render2d {

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
    Render2d::DrawList& GetDrawList() { return m_drawlist; }
    const Render2d::DrawList& GetDrawList() const { return m_drawlist; }
    void SetProgram(OglProgramPtr program = nullptr);
    // Tip: Don't use this in a loop where it assigns the same value every time. This kills batching.
    template <class T>
    void SetShaderParam(std::string_view name, const T& value) {
        if (m_params.program != nullptr)
            m_drawlist.shader_params.Set(m_params.program->GetUniformLocation(name), value);
    }
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
    void TextureRect(TexturePtr texture, glm::vec2 top_left, glm::vec2 size = glm::vec2(NAN));
    void Ellipse(uint32_t num_points, glm::vec2 top_left, glm::vec2 size);
    void Ellipse(uint32_t num_points, float x, float y, float w, float h) { Ellipse(num_points, glm::vec2(x, y), glm::vec2(w, h)); }
    /**
     * @param texture Current texture (can be `nullptr`)
     * @param size Ellipse size. The default value will use the texture's size.
     *  If `texture` is `nullptr`, nothing will be drawn.
     */
    void TextureEllipse(TexturePtr texture, uint32_t num_points, glm::vec2 top_left, glm::vec2 size = glm::vec2(NAN));
    inline void PushTransform(glm::mat3 tform) {
        if (!m_transforms.empty())
            tform = m_transforms.back() * tform;
        m_transforms.emplace_back(tform);
    }
    inline void PopTransform() {
        if (!m_transforms.empty())
            m_transforms.pop_back();
    }
private:
    inline void PushVertex(Vertex v) {
        glm::vec3 vec{v.x, v.y, 1.f};
        if (!m_transforms.empty())
            vec = m_transforms.back() * vec;
        v.x = vec.x, v.y = vec.y;
        m_drawlist.vertices.emplace_back(v);
    }
    void TextInternal(FontHandle font, glm::vec2 top_left, const void* begin, const void* end, uint8_t stride);
    /** Internal utility to add rectangle geometry */
    void RectUv(glm::vec2 xy, glm::vec2 size, glm::vec2 uv, glm::vec2 uv_wh);
    /** Internal utility to add ellipse geometry */
    void EllipseUv(uint32_t num_points, glm::vec2 xy, glm::vec2 size, glm::vec2 uv, glm::vec2 uv_wh);

    /** @return `value` if `value` is not NaN. Otherwise, `default_value` */
    static glm::vec2 VecOrDefault(glm::vec2 value, glm::vec2 default_value); 
    void SetTexture(TexturePtr texture);
    /**
     * @brief Add a new draw call that draws the last number of indices, `num_indices`, with the current clipping rect and texture.
     * In most cases, it's appended to a previous draw call with the same parameters.
     * @param num_indices The number of new vertices added. Must be a multiple of 3.
     */
    void AddDrawCall(uint32_t num_indices);
    /// @brief Get or create a draw call with the same params as `m_params`. This is always the last call.
    Render2d::DrawCall* GetDrawCall();

    glm::vec4 m_rgba = glm::vec4(1);
    DrawList m_drawlist;
    std::vector<glm::vec4> m_clip_stack;
    std::vector<glm::mat3> m_transforms;
    DrawCallParams m_params;
    // True if `m_params` has been modified since the last-created call
    bool m_dirty_params = false;
};

}