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
    void ResetColor() { m_color = glm::vec4(1.f); }

    void Segment(const Point& a, const Point& b);

    void Rect(const Point& a, const Point& b);

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
    glm::vec4 m_color = glm::vec4(1.f);
};

}