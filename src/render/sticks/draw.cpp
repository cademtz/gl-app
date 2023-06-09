#include "draw.hpp"
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/glm.hpp>

namespace sticks {

void Draw::Clear() {
    ResetColor();
    ResetTransform();
    m_drawlist.vertices.clear();
    m_drawlist.indices.clear();
    m_drawlist.calls.clear();
    m_current_draw_call = nullptr;
}

void Draw::SetTransform(const glm::mat3x3& transform) {
    if (m_transform != transform)
        OnDrawParamsChanged();
    m_transform = transform;
}

void Draw::Segment(const Point& a, const Point& b) {
    uint32_t old_index_count = m_drawlist.indices.size();

    // Clockwise radians from points `a` to `b`
    glm::mat3x3 t = Draw::GetLineTransform(a, b);
    float length = glm::length(b.pos-a.pos);
    
    constexpr std::array<uint32_t, 6> rect_indices = { 0,1,2,2,3,0 };
    constexpr std::array<glm::vec2, 4> rect_lr = {
        glm::vec2(0,-1), glm::vec2(1,-1),
        glm::vec2(1,1), glm::vec2(0,1)
    };
    
    for (int i = 0; i < rect_lr.size(); ++i) {
        const Point& p = i < 2 ? a : b;
        glm::vec3 pos = t * glm::vec3(rect_lr[i] * glm::vec2(length, p.radius), 1);
        m_drawlist.vertices.emplace_back(Vertex {
            pos.x, pos.y,
            0, 1,
            p.rgba.r, p.rgba.g, p.rgba.b, p.rgba.a,
            1
        });
    }

    for (uint32_t index_off : rect_indices) {
        m_drawlist.indices.push_back(old_index_count + index_off);
    }

    AddDrawCall(rect_indices.size());
}

void Draw::Raw(const Vertex* tri) {
    uint32_t start_index = m_drawlist.indices.size();
    for (size_t i = 0; i < 3; ++i) {
        m_drawlist.vertices.push_back(tri[i]);
        m_drawlist.indices.push_back(start_index + i);
    }
    AddDrawCall(3);
}

glm::mat3x3 Draw::GetLineTransform(const Point& a, const Point& b) {
    glm::mat3x3 t = glm::mat3x3(1.f);
    if (a.pos == b.pos)
        return t;
    
    glm::vec2 direction = b.pos - a.pos;
    float rotate = glm::tanh(direction.x / direction.y);

    t = glm::rotate(t, rotate);
    t = glm::translate(t, a.pos);
    return t;
}

void Draw::OnDrawParamsChanged() {
    m_current_draw_call = nullptr;
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
    assert(
        last_drawn_offset + num_indices == m_drawlist.indices.size()
        && "Indices are likely unused or being re-used on accident. Make sure num_indices is correct!"
    );
    call->index_count += num_indices;
}

DrawCall* Draw::GetDrawCall() {
    if (m_current_draw_call)
        return m_current_draw_call;
    if (m_drawlist.calls.empty())
        m_drawlist.calls.emplace_back(DrawCall{0, 0, m_transform});
    return m_current_draw_call = &m_drawlist.calls.back();
}

}