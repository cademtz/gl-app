#pragma once
#include <render/rendergui.h>
#include "programglsl.h"

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
    GLuint m_glVertexBuffer;
    GLuint m_glIndexBuffer;
    GLuint m_uploadedIndices = 0;

    GLuint m_mPixelToNormalized;
    GLuint m_vPos;
    GLuint m_vUv;
    GLuint m_vCol;
};