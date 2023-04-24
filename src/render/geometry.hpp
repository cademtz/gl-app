#pragma once
#include "glm/fwd.hpp"
#include <stdint.h>
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include <memory>
#include "bake.hpp"
#include "texture.hpp"

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
    std::shared_ptr<CTexture> texture = nullptr;
};

struct Geometry3d
{
    std::vector<Vertex3d> vertices;
    std::vector<vertexId_t> indices;
    std::shared_ptr<CTexture> texture = nullptr;
};

/**
 * @brief Create primitive 2D geometry
 */
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

    const glm::vec4& GetColor() const { return m_rgba; }
    void SetColor(const glm::vec4& Rgba) { m_rgba = Rgba; }
    void SetColor(float R, float G, float B, float A = 1.0f) {
        m_rgba = { R, G, B, A };
    }
    
    EAlign GetAlign() const { return m_align; }
    void SetAlign(EAlign Align) { m_align = Align; }

    // TODO: Get/SetTransform matrix instead (and apply after corner/center align)
    float GetRotation() const { return m_rotate; }
    void SetRotation(float Radians) { m_rotate = Radians; }

    /**
     * @brief Sets alignment, rotation, and other values to their defaults
     */
    void ClearTransform() {
        m_rotate = 0, m_align = EAlign::Align_TopLeft;
    }

    void Rectangle(float X, float Y, float Width, float Height);
    void Ellipse(float X, float Y, float Width, float Height);

protected:
    /**
     * @brief This UV is used for any "untextured" shape.
     * Set this to a white pixel in the active texture.
     */
    glm::vec2 m_default_uv = { 0, 0 };
    Geometry2d& m_geom;

private:
    glm::mat3x3 CreateTransform(int X, int Y, int Width, int Height);

    EAlign m_align = Align_TopLeft;
    float m_rotate = 0;
    
    glm::vec4 m_rgba = { 0, 0, 0, 1 };
    uint32_t m_curveDensity = 32;
};

/**
 * @brief Create primitive 2D geometry with texture
 */
class TexturedBuilder2d : public PrimitiveBuilder2d
{
public:
    /**
     * @brief Add a textured rectangle with the texture stretched to fit
     */
    void TexturedRect(glm::vec2 origin, glm::vec2 size, uint32_t tex_id);

    /**
     * @brief Add a textured rectangle with the texture's size in pixels
     */
    void TexturedRect(glm::vec2 origin, uint32_t tex_id) {
        CRectPacker::Rect rect;
        GetTextureRect(tex_id, &rect);
        return TexturedRect(origin, glm::vec2(rect.w, rect.h), tex_id);
    }

    /**
     * @brief Puts the entire atlas texture on screen.
     * For debugging purposes.
     */
    void AtlasRect(glm::vec2 origin);

    void SetDefaultUv(glm::vec2 uv) {
        m_default_uv = uv;
    }

    void GetTextureRect(uint32_t id, CRectPacker::Rect* out_rect) {
        m_atlas->GetRect(id, out_rect);
    }

private:
    std::shared_ptr<CRectPacker> m_atlas;
};