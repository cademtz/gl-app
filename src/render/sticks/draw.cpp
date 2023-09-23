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

    // Clockwise radians from points `a` to `b`
    glm::mat3x3 t = Draw::GetLineTransform(a, b);
    float length = glm::distance(a.pos, b.pos);
    
    constexpr std::array<uint32_t, 6> rect_indices = { 1, 3, 2, 2, 0, 1 };
    // Left-to-right, vertically-centered rectangle
    constexpr std::array<glm::vec2, 4> rect_lr = {
        glm::vec2(0,-1), glm::vec2(0,1),
        glm::vec2(1,-1), glm::vec2(1,1)
    };
    
    uint32_t start_index = m_drawlist.vertices.size();
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
        m_drawlist.indices.push_back(start_index + index_off);
    }

    // Draw cap
    for (int i = 0; i < 2; ++i) {
        const Point& p = i == 0 ? a : b;
        const float x_off = i == 0 ? 0.f : length;
        const float x_dir = i == 0 ? -1.f : 1.f;

        uint32_t start_index = m_drawlist.vertices.size();

        switch (p.cap) {
        case SegmentCap::CIRCLE:
        {
            glm::vec2 top = glm::vec3(x_off * x_dir, p.radius, 1);
            glm::vec2 tip = glm::vec3((x_off + p.radius) * x_dir, 0, 1);
            glm::vec2 bottom = glm::vec3(x_off * x_dir, -p.radius, 1);

            // Push untranslated vertices now, translate later
            
            // The fill shape (triangle extends horizontal towards tip like a spike)
            Vertex base_vertex = {
                top.x, top.y,
                0, 1,
                p.rgba.r, p.rgba.g, p.rgba.b, p.rgba.b,
                1
            };
            m_drawlist.vertices.push_back(base_vertex);
            base_vertex.x = tip.x, base_vertex.y = tip.y;
            m_drawlist.vertices.push_back(base_vertex);
            base_vertex.x = bottom.x, base_vertex.y = bottom.y;
            m_drawlist.vertices.push_back(base_vertex);
            
            // The top curve (rests on top-half of spike to add curvature)
            base_vertex.x = top.x, base_vertex.y = top.y, base_vertex.u = base_vertex.v = 0.f; // anchor
            m_drawlist.vertices.push_back(base_vertex);
            base_vertex.x = tip.x, base_vertex.y = top.y, base_vertex.u = 0.5f, base_vertex.v = 0.f;
            m_drawlist.vertices.push_back(base_vertex);
            base_vertex.x = tip.x, base_vertex.y = tip.y, base_vertex.u = base_vertex.v = 1.f;
            m_drawlist.vertices.push_back(base_vertex);

            // The bottom curve (mirror top curve vertically)
            for (uint32_t end = m_drawlist.vertices.size(), index = end - 3; index < end; ++index) {
                base_vertex = m_drawlist.vertices[index];
                base_vertex.y *= -1;
                m_drawlist.vertices.push_back(base_vertex);
            }

            // Translate all vertices and add indices
            for (uint32_t index = start_index; index < m_drawlist.vertices.size(); ++index) {
                Vertex& vertex = m_drawlist.vertices[index];
                glm::vec2 pos = t * glm::vec3(vertex.x, vertex.y, 1);
                vertex.x = pos.x, vertex.y = pos.y;
                m_drawlist.indices.push_back(index);
            }
            
            break;
        }
        case SegmentCap::SQUARE:
            break;
        case SegmentCap::NONE:
            break;
        }
    }

    AddDrawCall(m_drawlist.indices.size() - start_index);
}

void Draw::Raw(const Vertex* tri) {
    uint32_t start_index = m_drawlist.vertices.size();
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
    float rotate = glm::atan(direction.y / direction.x);

    t = glm::translate(t, a.pos);
    t = glm::rotate(t, rotate);
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

    // Check if we already have a draw call with the same params
    if (!m_drawlist.calls.empty()) {
        DrawCall* last = &m_drawlist.calls.back();
        if (last->transform == m_transform)
            return m_current_draw_call = last;
    }

    m_drawlist.calls.emplace_back(DrawCall{0, 0, m_transform});
    return m_current_draw_call = &m_drawlist.calls.back();
}

}