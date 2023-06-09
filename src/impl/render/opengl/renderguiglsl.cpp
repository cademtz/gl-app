#include "renderguiglsl.hpp"
#include <platform.hpp>
#include <render/gui/rendergui.hpp>
#include <render/gui/drawlist.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include "opengl.hpp"
#include "texture.hpp"

// Debugging
#include <iostream>

static const char* VERT_SHADER_SRC =
_IMPL_GLSL_VERSION_HEADER
"uniform mat4 pixel_to_normalized;"
"uniform vec2 texel_to_normalized;"
"in vec2 in_pos;"
"in vec2 in_uv;"
"in vec4 in_color;"
"out vec2 frag_uv;"
"out vec4 frag_color;"

"void main() {"
"   gl_Position = pixel_to_normalized * vec4(in_pos, 0.0, 1.0);"
"   frag_uv = texel_to_normalized * in_uv;"
"   frag_color = in_color;"
"}";

static const char* FRAG_SHADER_SRC =
_IMPL_GLSL_VERSION_HEADER
"precision mediump float;"
"in vec2 frag_uv;"
"in vec4 frag_color;"
"out vec4 final_frag_color;"

"uniform sampler2D in_texture;"

"void main() {"
"   final_frag_color = texture(in_texture, frag_uv).rgba * frag_color;"
"}";

namespace gui {
std::shared_ptr<RenderGui> RenderGui::GetInstance() {
    static auto ptr = std::make_shared<RenderGuiGlsl>();
    return ptr;
}
}

RenderGuiGlsl::RenderGuiGlsl() : RenderGui("RenderGuiGlsl") {
   if (!Init())
       PLATFORM_ERROR("Failed to initialize RenderGuiGlsl");
}

RenderGuiGlsl::~RenderGuiGlsl() {
    if (m_vertex_buffer)
        glDeleteBuffers(1, &m_vertex_buffer);
    if (m_index_buffer)
        glDeleteBuffers(1, &m_index_buffer);
    if (m_array_object)
        glDeleteVertexArrays(1, &m_array_object);
}

bool RenderGuiGlsl::Init() {
    uint8_t white_px[4] = { 255, 255, 255, 255 };
    m_default_texture = Texture::Create(TextureInfo(TextureFormat::RGBA_8_32, 1, 1), white_px);

    static CShaderGlsl vert_shader(GL_VERTEX_SHADER, VERT_SHADER_SRC);
    static CShaderGlsl frag_shader(GL_FRAGMENT_SHADER, FRAG_SHADER_SRC);

    if (!vert_shader.Compile() ||
        !frag_shader.Compile())
        return false;
    
    if (!m_glProgram.AttachShader(vert_shader) ||
        !m_glProgram.AttachShader(frag_shader)
    ) {
        PLATFORM_ERROR("Failed to attach shaders");
        return false;
    }

    if (!m_glProgram.Link()) {
        PLATFORM_ERROR("Failed to link shaders");
        return false;
    }

    m_pixel_to_normalized = m_glProgram.GetUniformLocation("pixel_to_normalized");
    m_texel_to_normalized = m_glProgram.GetUniformLocation("texel_to_normalized");
    m_in_pos = m_glProgram.GetAttribLocation("in_pos");
    m_in_uv = m_glProgram.GetAttribLocation("in_uv");
    m_in_color = m_glProgram.GetAttribLocation("in_color");

    glGenBuffers(1, &m_vertex_buffer);
    glGenBuffers(1, &m_index_buffer);
    glGenVertexArrays(1, &m_array_object);

    // While VAO `m_array_object` is active, the following binds and attrib pointers are stored in the object (m_array_object)
    glBindVertexArray(m_array_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);

    glEnableVertexAttribArray(m_in_pos);
    glEnableVertexAttribArray(m_in_uv);
    glEnableVertexAttribArray(m_in_color);

    glVertexAttribPointer(m_in_pos, 2, GL_FLOAT, GL_FALSE, sizeof(gui::Vertex), (void*)0);
    glVertexAttribPointer(m_in_uv, 2, GL_FLOAT, GL_FALSE, sizeof(gui::Vertex), (void*)(2 * sizeof(float)));
    glVertexAttribPointer(m_in_color, 4, GL_FLOAT, GL_FALSE, sizeof(gui::Vertex), (void*)(4 * sizeof(float)));
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return true;
}

void RenderGuiGlsl::UploadDrawData(const gui::DrawList& list) {
    m_drawlist = &list;
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
    
    glBufferData(GL_ARRAY_BUFFER,
        list.vertices.size() * sizeof(list.vertices[0]),
        list.vertices.data(), GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        list.indices.size() * sizeof(list.indices[0]),
        list.indices.data(), GL_STREAM_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void RenderGuiGlsl::Render() {
    float width = ScreenWidth();
    float height = ScreenHeight();
    float aspect = width / height;
    
    glViewport(0, 0, width, height);
    glUseProgram(m_glProgram.GlHandle());
    glBindVertexArray(m_array_object);

    glm::mat4x4 m = glm::mat4x4(1.f);
    m = glm::ortho<float>(0, width, height, 0, 1, -1);
    glUniformMatrix4fv(m_pixel_to_normalized, 1, GL_FALSE, (const GLfloat*)&m[0][0]);

    for (const gui::DrawCall& call : m_drawlist->calls) {
        // Bind texture
        Texture::Ptr current_tex = call.texture;
        if (!current_tex)
            current_tex = m_default_texture;
        std::shared_ptr<COpenGLTexture> gl_tex = std::dynamic_pointer_cast<COpenGLTexture>(current_tex);
        glBindTexture(GL_TEXTURE_2D, gl_tex->GetId());
        glUniform2f(m_texel_to_normalized, 1.f / gl_tex->GetWidth(), 1.f / gl_tex->GetHeight());

        // Clip geometry
        if (glm::isnan(call.clip_rect.x))
            glDisable(GL_SCISSOR_TEST);
        else {
            glEnable(GL_SCISSOR_TEST);
            // glScissor uses y=0 as the bottom of the screen
            
            glm::vec<4, int32_t> irect;
            for (int i = 0; i < 4; ++i)
                irect[i] = (int32_t)glm::round(call.clip_rect[i]);

            float new_y = height - irect.y - irect[3];
            glScissor(irect.x, new_y, irect[2], irect[3]);
        }

        glDrawElements(GL_TRIANGLES, call.index_count, GL_UNSIGNED_INT, (void*)(call.index_offset * sizeof(GLuint)));
    }

    glDisable(GL_SCISSOR_TEST);
    glBindVertexArray(0);
    glUseProgram(0);
}
