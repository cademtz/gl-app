#pragma once
#include "render/sticks/drawlist.hpp"
#include "sticks.hpp"
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <glm/mat3x3.hpp>

namespace sticks {

class Draw {
public:
    Draw() {}

    void Clear();

    void SetColor(const glm::vec4& rgba) { m_color = rgba; }
    void SetColor(float r, float g, float b, float a = 1.f) { SetColor(glm::vec4(r,g,b,a)); }
    void ResetColor() { SetColor(glm::vec4(1.f)); }

    void SetTransform(const glm::mat3x3& transform);
    void ResetTransform() { SetTransform(glm::mat3x3(1.f)); }

    void Segment(const Point& a, const Point& b);

    void Rect(const Point& a, const Point& b);

    void Raw(const Vertex* tri);

    //DrawList& GetDrawList() { return m_drawlist; }
    const DrawList& GetDrawList() const { return m_drawlist; }

private:
    /**
     * @brief Creates a transform such that `(0, 0)` translates to to `from`,
     *  and positive values along the X axis moves towards `to`.
     */
    static glm::mat3x3 GetLineTransform(const Point& from, const Point& to);
    
    void OnDrawParamsChanged();
    void AddDrawCall(uint32_t num_indices);
    DrawCall* GetDrawCall();

    DrawList m_drawlist;
    DrawCall* m_current_draw_call = nullptr;
    glm::mat3x3 m_transform = glm::mat3x3(1.f);
    glm::vec4 m_color = glm::vec4(1.f);
};

}