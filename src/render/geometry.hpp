#pragma once
#include <stdint.h>
#include <vector>
#include <array>
#include <glm/glm.hpp>

typedef int32_t vertexId_t;

struct Vertex2d
{
    float x, y;
    float u, v;
    float r, g, b, a;
};

struct Vertex3d
{
    float x, y, z;
    float u, v;
    float r, g, b, a;
};

struct Geometry2d
{
    std::vector<Vertex2d> vertices;
    std::vector<vertexId_t> indices;
};

struct Geometry3d
{
    std::vector<Vertex3d> vertices;
    std::vector<vertexId_t> indices;
};

class PrimitiveBuilder2d
{
public:
    enum EAlign
    {
        Align_Top,
        Align_TopRight,
        Align_Right,
        Align_BottomRight,
        Align_Bottom,
        Align_BottomLeft,
        Align_Left,
        Align_TopLeft,
        Align_Center
    };

    PrimitiveBuilder2d(Geometry2d& Geometry) : m_geom(Geometry) {}

    const std::array<float, 4>& GetColor() const { return m_rgba; }
    void SetColor(const float* Rgba) { m_rgba = *(std::array<float, 4>*)Rgba; }
    void SetColor(const std::array<float, 4>& Rgba) { m_rgba = Rgba; }
    void SetColor(float R, float G, float B, float A = 1.0f) {
        m_rgba = { R, G, B, A };
    }
    
    EAlign GetAlign() const { return m_align; }
    void SetAlign(EAlign Align) { m_align = Align; }

    // TODO: Get/SetTransform matrix instead (and apply after corner/center align)
    float GetRotation() const { return m_rotate; }
    void SetRotation(float Radians) { m_rotate = Radians; }

    void Rectangle(float X, float Y, float Width, float Height);
    void Ellipse(float X, float Y, float Width, float Height);

private:

    glm::mat3x3 CreateTransform(int X, int Y, int Width, int Height);

    EAlign m_align = Align_TopLeft;
    float m_rotate = 0;
    
    uint32_t m_curveDensity = 32;
    std::array<float, 4> m_rgba = { 0, 0, 0, 1 };
    Geometry2d& m_geom;
};