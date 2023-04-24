#pragma once
#include <render/rendergui.hpp>
#include "opengl.hpp"
#include "programglsl.hpp"
#include "render/texture.hpp"

class CRenderGuiGlsl : public CRenderGui
{
public:
    CRenderGuiGlsl() : CRenderGui("CRenderGuiGlsl") { }
    ~CRenderGuiGlsl();

    bool Init() override;
    void UploadGeometry(const Geometry2d& Geometry) override;
    void Render() override;

private:
    CProgramGlsl m_glProgram;
    CTexture::Ptr m_default_texture = nullptr;
    CTexture::Ptr m_current_texture = nullptr;
    GLuint m_glVertexBuffer;
    GLuint m_glIndexBuffer;
    GLuint m_uploadedIndices = 0;

    GLuint m_mPixelToNormalized;
    GLuint m_mTexelToNormalized;
    GLuint m_vPos;
    GLuint m_vUv;
    GLuint m_vCol;
};