#pragma once
#include <render/gui/rendergui.hpp>
#include <render/texture.hpp>
#include "programglsl.hpp"
#include "opengl.hpp"

namespace gui { class DrawList; }

class RenderGuiGlsl : public gui::RenderGui {
public:
    RenderGuiGlsl();
    ~RenderGuiGlsl();

    void UploadDrawData(const gui::DrawList& list) override;
    void Render() override;

private:
    bool Init();

    Texture::Ptr m_default_texture = nullptr;
    const gui::DrawList* m_drawlist;

    CProgramGlsl m_glProgram;
    GLuint m_vertex_buffer;
    GLuint m_index_buffer;
    GLuint m_array_object; // Stores the layout/pointers of attributes in buffers

    GLuint m_pixel_to_normalized;
    GLuint m_texel_to_normalized;
    GLuint m_in_pos;
    GLuint m_in_uv;
    GLuint m_in_color;
};