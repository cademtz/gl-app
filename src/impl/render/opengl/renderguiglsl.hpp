#pragma once
#include <render/gui/rendergui.hpp>
#include "opengl.hpp"
#include "programglsl.hpp"
#include "render/texture.hpp"

namespace gui { class DrawList; }

class RenderGuiGlsl : public gui::RenderGui
{
public:
    RenderGuiGlsl() : RenderGui("RenderGuiGlsl") { }
    ~RenderGuiGlsl();

    bool Init() override;
    void UploadDrawData(const gui::DrawList& list) override;
    void Render() override;

private:
    Texture::Ptr m_default_texture = nullptr;
    const gui::DrawList* m_drawlist;

    CProgramGlsl m_glProgram;
    GLuint m_glVertexBuffer;
    GLuint m_glIndexBuffer;

    GLuint m_mPixelToNormalized;
    GLuint m_vTexelToNormalized;
    GLuint m_vPos;
    GLuint m_vUv;
    GLuint m_vCol;
};