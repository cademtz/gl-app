#include "draw.hpp"
#include "font.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/glm.hpp"
#include "glm/ext/scalar_constants.hpp"
#include <resources/resource.hpp>
#include <array>

// Debugging
#include <iostream>

namespace gui {

static constexpr std::array<uint32_t, 6> rect_indices = { 0,1,2,2,3,0 };
static constexpr std::array<glm::vec2, 4> rect_wh = { glm::vec2{0.f,0.f}, {1,0}, {1,1}, {0,1} };

void _FontHandle::OnDeath() { m_parent.NotifyDeath(this); }

Draw::Draw() {}

void Draw::Clear() {
    m_drawcall = nullptr;
    m_drawlist.indices.clear();
    m_drawlist.vertices.clear();
    m_drawlist.calls.clear();

    m_clip_stack.clear();
    m_clip = NO_CLIP;

    ResetColor();
}

void Draw::SetColor(const glm::vec4& rgba) { m_rgba = rgba; }

void Draw::PushClip(glm::vec4 new_clip) {
    if (!glm::isnan(new_clip.x) && !glm::isnan(m_clip.x)) {
        // Clamp `new_clip` within the boundaries of the current `m_clip_rect`
        // using { x0,y0, x1,y1 } format
        glm::vec4 old_corners = glm::vec4(m_clip[0], m_clip[1], m_clip[0] + m_clip[2], m_clip[1] + m_clip[3]);
        glm::vec4 new_corners = glm::vec4(new_clip[0], new_clip[1], new_clip[0] + new_clip[2], new_clip[1] + new_clip[3]);

        // Top-left
        for (uint8_t tl = 0; tl < 2; ++tl) {
            if (new_corners[tl] < old_corners[tl])
                new_corners[tl] = old_corners[tl];
        }

        // Bottom-right
        for (uint8_t br = 2; br < 4; ++br) {
            if (new_corners[br] > old_corners[br])
                new_corners[br] = old_corners[br];
        }

        // Use { x, y, w, h } format again
        new_clip = new_corners;
        new_clip[2] -= new_clip[0];
        new_clip[3] -= new_clip[1];
    }

    // If the clipping will change
    if (new_clip != m_clip)
        OnDrawParamsChanged();

    m_clip_stack.emplace_back(m_clip);
    m_clip = new_clip;
}

void Draw::PopClip() {
    assert(!m_clip_stack.empty() && "Too many pops");
    glm::vec4 prev_clip = m_clip_stack.back();
    if (prev_clip != m_clip)
        OnDrawParamsChanged();
    m_clip = prev_clip;
    m_clip_stack.pop_back();
}

Draw::Font Draw::CreateFont(FontBakeConfig&& config) {
    CResource::Ptr res = CResource::LoadSynchronous(config.url);
    if (!res) {
        std::cout << "res == nullptr" << std::endl;
        return nullptr;
    }
    
    std::optional<TrueType> tt = TrueType::FromTrueType(res);
    if (!tt) {
        std::cout << "failed to parse truetype" << std::endl;
        return nullptr;
    }
    
    Font font = std::make_shared<_FontHandle>(*this);
    m_fonts.emplace(std::make_pair(font.get(), FontAtlas(*tt, std::move(config))));
    return font;
}

void Draw::Codepoint(Font font, codepoint_t codepoint, glm::vec2 top_left) {
    const FontAtlas* atlas = GetFontAtlas(font);
    if (!atlas)
        return;
    
    const FontGlyphInfo* glyph = atlas->GetCodepointMap().FindGlyph(codepoint);
    if (glyph == nullptr) {
        std::cout << "glyph == nullptr while looking up " << codepoint << std::endl;
        return;
    }
    
    FontAtlas::GlyphRect rect;
    bool ok = atlas->GetGlyphTextureRect(glyph->id, &rect);
    assert(ok && "This glyph wasn't mapped");

    if (rect.w == 0 || rect.h == 0) {
        std::cout << "empty glyph" << std::endl;
        return;
    }
    
    SetTexture(atlas->GetTexture());
    
    uint32_t index_off = m_drawlist.vertices.size();
    RectUv(top_left, glm::vec2(rect.w, rect.h), glm::vec2(rect.x, rect.y), glm::vec2(rect.w, rect.h));
    
    AddDrawCall(rect_indices.size());
}

void Draw::TextUnicode(Font font, glm::vec2 top_left, std::basic_string_view<uint32_t> text) {
    TextInternal(font, top_left, &text.front(), &text.back() + 1, sizeof(text[0]));
}

void Draw::TextAscii(Font font, glm::vec2 top_left, std::string_view text) {
    TextInternal(font, top_left, &text.front(), &text.back() + 1, sizeof(text[0]));
}

void Draw::TextInternal(Font font, glm::vec2 top_left, const void* begin, const void* end, uint8_t stride) {
    const FontAtlas* atlas = GetFontAtlas(font);
    if (!atlas)
        return;
    
    const FontLineMetrics& line_metrics = atlas->GetLineMetrics();
    float line_ascent = line_metrics.line_y0 * atlas->GetScale();
    float line_descent = line_metrics.line_y1 * atlas->GetScale();
    float line_gap = line_metrics.gap * atlas->GetScale();

    float hcursor = top_left.x;
    float vcursor = top_left.y + line_ascent;
    uint32_t indices_start = m_drawlist.indices.size();

    SetTexture(atlas->GetTexture());

    uint32_t prev_glyph = 0;
    for (uint8_t* next = (uint8_t*)begin; next < end; next += stride) {
        uint32_t cp = 0;
        switch (stride) {
        case 1: cp = *next; break;
        case 4: cp = *(uint32_t*)next; break;
        default:
        assert(0 && "Invalid character size");
        }

        if (cp == '\n') {
            hcursor = top_left.x;
            vcursor += line_ascent - line_descent + line_gap;
            prev_glyph = 0;
            continue;
        }

        const FontGlyphInfo* glyph = atlas->GetCodepointMap().FindGlyph(cp);
        if (!glyph)
            continue;
        
        if (prev_glyph != 0) {
            const int32_t* kern = atlas->GetCodepointMap().FindKerning(prev_glyph, glyph->id);
            if (kern)
                hcursor += *kern * atlas->GetScale();
        }
        prev_glyph = glyph->id;

        if (!glyph->metrics.IsEmpty()) {
            FontAtlas::GlyphRect glyph_rect;
            bool ok = atlas->GetGlyphTextureRect(glyph->id, &glyph_rect);
            assert(ok && "Could not find texture rect for glyph");

            //float glyph_height = glyph->metrics.GetHeight(atlas->GetScale());
            glm::vec2 glyph_pos = glm::vec2(hcursor + glyph->metrics.x0 * atlas->GetScale(), vcursor - glyph->metrics.y1 * atlas->GetScale());
            glm::vec2 glyph_uv = glm::vec2(glyph_rect.x, glyph_rect.y);
            glm::vec2 glyph_tex_size = glm::vec2(glyph_rect.w, glyph_rect.h);
            glm::vec2 glyph_pixel_pos = glm::round(glyph_pos);

            RectUv(glyph_pixel_pos, glyph_tex_size, glyph_uv, glyph_tex_size);
        }

        hcursor += glyph->metrics.next_x_offset * atlas->GetScale();
    }

    uint32_t num_indices = m_drawlist.indices.size() - indices_start;
    AddDrawCall(num_indices);
}

void Draw::DebugFontAtlas(Font font, glm::vec2 top_left, glm::vec2 size) {
    FontAtlas* atlas = GetFontAtlas(font);
    if (atlas)
        TextureRect(atlas->GetTexture(), top_left, size);
}

void Draw::Rect(glm::vec2 top_left, glm::vec2 size) {
    TextureRect(nullptr, top_left, size);
}

void Draw::TextureRect(Texture::Ptr texture, glm::vec2 top_left, glm::vec2 size) {
    glm::vec2 tex_size = glm::vec2(0);
    if (texture != nullptr)
        tex_size = glm::vec2(texture->GetWidth(), texture->GetHeight());

    if (glm::isnan(size.x)) {
        if (tex_size.x == 0 || tex_size.y == 0)
            return;
        size = tex_size;
    }

    SetTexture(texture);
    RectUv(top_left, size, glm::vec2(0), tex_size);
    AddDrawCall(rect_indices.size());
}

void Draw::Ellipse(uint32_t num_points, glm::vec2 top_left, glm::vec2 size) {
    TextureEllipse(nullptr, num_points, top_left, size);
}

void Draw::TextureEllipse(Texture::Ptr texture, uint32_t num_points, glm::vec2 top_left, glm::vec2 size) {
    glm::vec2 tex_size = glm::vec2(0);
    if (texture != nullptr)
        tex_size = glm::vec2(texture->GetWidth(), texture->GetHeight());

    if (glm::isnan(size.x)) {
        if (tex_size.x == 0 || tex_size.y == 0)
            return;
        size = tex_size;
    }
    
    uint32_t prev_num_indices = m_drawlist.indices.size();

    SetTexture(texture);
    EllipseUv(num_points, top_left, size, glm::vec2(0), tex_size);
    AddDrawCall(m_drawlist.indices.size()- prev_num_indices);
}

void Draw::NotifyDeath(_FontHandle* font) { m_fonts.erase(font); }

void Draw::RectUv(glm::vec2 xy, glm::vec2 size, glm::vec2 uv, glm::vec2 uv_wh) {
    uint32_t index_off = m_drawlist.vertices.size();
    
    for (const glm::vec2& wh : rect_wh) {
        m_drawlist.vertices.emplace_back(Vertex{
            xy.x + size.x * wh.x, xy.y + size.y * wh.y,
            uv.x + uv_wh.x * wh.x , uv.y + uv_wh.y * wh.y,
            m_rgba[0], m_rgba[1], m_rgba[2], m_rgba[3]
        });
    }

    for (uint32_t index : rect_indices)
        m_drawlist.indices.push_back(index + index_off);
}

void Draw::EllipseUv(uint32_t num_points, glm::vec2 xy, glm::vec2 size, glm::vec2 uv, glm::vec2 uv_wh) {
    assert(num_points >= 3 && "Ellipse must have 3 or more points");
    if (num_points < 3)
        return;

    uint32_t index_off = m_drawlist.vertices.size();
    glm::vec2 radii = size / 2.f;
    glm::vec2 uv_radii = uv_wh / 2.f;
    glm::vec2 centered_offset = xy + radii;
    glm::vec2 uv_centered_offset = uv + uv_radii;

    for (uint32_t p = 0; p < num_points; ++p) {
        float angle = 2.f * glm::pi<float>() * p / num_points;
        glm::vec2 rotate = glm::vec2(glm::cos(angle), glm::sin(angle));
        glm::vec2 point = radii * rotate + centered_offset;
        glm::vec2 uv_point = uv_radii * rotate + uv_centered_offset;
        
        m_drawlist.vertices.emplace_back(Vertex{
            point.x, point.y,
            uv_point.x, uv_point.y,
            m_rgba.r, m_rgba.g, m_rgba.b, m_rgba.a
        });
    }
    
    for (uint32_t index = 1; index < num_points - 1; ++index)
        m_drawlist.indices.insert(m_drawlist.indices.end(), { 0 + index_off, index + index_off, index + index_off + 1 });
}

glm::vec2 Draw::VecOrDefault(glm::vec2 value, glm::vec2 default_value) {
    return glm::isnan(value.x) ? default_value : value;
}

FontAtlas* Draw::GetFontAtlas(Font font) {
    auto it = m_fonts.find(font.get());
    if (it == m_fonts.cend())
        return nullptr;
    return &it->second;
}

void Draw::SetTexture(Texture::Ptr texture) {
    // If the texture will change
    if (m_texture != texture)
        OnDrawParamsChanged();
    
    m_texture = texture;
}

void Draw::OnDrawParamsChanged() {
    m_drawcall = nullptr;
}

void Draw::AddDrawCall(uint32_t num_indices) {
    if (num_indices == 0)
        return;
    
    assert((num_indices % 3 == 0) && "num_indices must be a multiple of 3 to create triangles");
    assert(num_indices <= m_drawlist.indices.size() && "num_indices is greater than the total available indices");
    
    DrawCall* call = GetDrawCall();

    if (call->index_count == 0)
        call->index_offset = m_drawlist.indices.size() - num_indices;
    
    uint32_t last_drawn_offset = call->index_offset + call->index_count;
    //std::cout << "last end: " << last_drawn_offset << ", total indices: " << m_drawlist.indices.size();
    //std::cout << " new end: " << last_drawn_offset + num_indices;
    //std::cout << std::endl;
    assert(
        last_drawn_offset + num_indices == m_drawlist.indices.size()
        && "Indices are likely unused or being re-used on accident. Make sure num_indices is correct!"
    );
    call->index_count += num_indices;
}

DrawCall* Draw::GetDrawCall() {
    if (m_drawcall)
        return m_drawcall;
    
    if (!m_drawlist.calls.empty()) {
        // Check if we can just append data to the last draw call
        DrawCall* prev = &m_drawlist.calls.back();
        if (prev->clip_rect == m_clip && prev->texture == m_texture) {
            m_drawcall = prev;
            return prev;
        }
    }

    // Start a new call
    m_drawcall = &m_drawlist.calls.emplace_back(DrawCall{
        0, 0, m_clip, m_texture
    });
    return m_drawcall;
}

}