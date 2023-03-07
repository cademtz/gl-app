#pragma once
#include "render.hpp"
#include "geometry.hpp"

// Renders 2D geometry starting from the top-left (0, 0) and ending at the number of pixels on screen.
class CRenderGui : public CRender
{
public:
    CRenderGui(const std::string& DebugName) : CRender(DebugName) { }

    virtual void UploadGeometry(const Geometry2d& Geometry) = 0;

    float ScreenWidth() const { return m_screenWidth; }
    float ScreenHeight() const { return m_screenHeight; }
    void SetScreenSize(float Width, float Height) {
        m_screenWidth = Width, m_screenHeight = Height;
    }

private:
    float m_screenWidth = 0;
    float m_screenHeight = 0;
};