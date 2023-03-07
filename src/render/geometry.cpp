#include "geometry.hpp"
#include <memory.h>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/ext/scalar_constants.hpp>

static const glm::vec2 align_wh[] = {
    { -0.5f ,  0    }, // Top
    { -1    ,  0    }, // TopRight
    { -1    , -0.5f }, // Right
    { -1    , -1    }, // BottomRight
    { -0.5f , -1    }, // Bottom
    {  0    , -1    }, // BottomLeft
    {  0    , -0.5f }, // Left
    {  0    ,  0    }, // TopLeft
    { -0.5f , -0.5f }, // Center
};
static const std::array<vertexId_t, 6> rect_indices = {
    0, 1, 2, 2, 3, 0
};

void PrimitiveBuilder2d::Rectangle(float X, float Y, float Width, float Height)
{
    vertexId_t startId = m_geom.vertices.size();
    std::array<glm::vec2, 4> rect = {
        glm::vec2{ 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 }
    };

    m_geom.vertices.reserve(startId + rect.size());
    m_geom.indices.reserve(m_geom.indices.size() + rect_indices.size());

    glm::mat3x3 t = CreateTransform(X, Y, Width, Height);

    for (int i = 0; i < rect.size(); ++i)
    {
        glm::vec3 point = glm::vec3(rect[i][0], rect[i][1], 1);
        point = t * point;

        m_geom.vertices.emplace_back(Vertex2d{
            point[0], point[1], 0,0,
            m_rgba[0], m_rgba[1], m_rgba[2], m_rgba[3]
        });
    }

    for (vertexId_t id : rect_indices)
        m_geom.indices.push_back(id + startId);
}

void PrimitiveBuilder2d::Ellipse(float X, float Y, float Width, float Height)
{
    assert(m_curveDensity >= 3 && "Cannot form 2D ellipse with fewer than 3 points");

    vertexId_t m_startId = m_geom.vertices.size();

    m_geom.vertices.reserve(m_startId + m_curveDensity);
    m_geom.indices.reserve(m_geom.indices.size() + (m_curveDensity - 2) * 3);

    glm::mat3x3 t = CreateTransform(X, Y, Width, Height);

    for (uint32_t i = 0; i < m_curveDensity; ++i)
    {
        float angle = 2 * glm::pi<float>() * ((float)i / m_curveDensity);
        glm::vec3 point = { (cosf(angle) + 1) / 2, (sinf(angle) + 1) / 2, 1 };
        point = t * point;
        m_geom.vertices.emplace_back(Vertex2d{
            point[0], point[1], 0,0,
            m_rgba[0], m_rgba[1], m_rgba[2], m_rgba[3]
        });
    }

    for (int i = 1; i < m_curveDensity - 1; ++i)
    {
        std::array<vertexId_t, 3> inds = { m_startId, m_startId + i, m_startId + i + 1 };
        m_geom.indices.insert(m_geom.indices.end(), inds.begin(), inds.end());
    }
}

glm::mat3x3 PrimitiveBuilder2d::CreateTransform(int X, int Y, int Width, int Height)
{
    glm::mat3x3 t = glm::mat3x3(1.f);
    t = glm::translate(t, glm::vec2(X, Y));
    t = glm::rotate(t, -m_rotate);
    t = glm::scale(t, glm::vec2(Width, Height));
    t = glm::translate(t, align_wh[m_align]);
    return t;
}
