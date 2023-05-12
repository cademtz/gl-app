#include "renderguiglsl.hpp"
#include "opengl.hpp"
#include "texture.hpp"
#include <render/gui/drawlist.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

// Debugging
#include <iostream>

static const char* vertShader_src =
_IMPL_GLSL_VERSION_HEADER
"uniform mat4 mPixelToNormalized;"
"uniform vec2 vTexelToNormalized;"
"in vec2 vPos;"
"in vec2 vUv;"
"in vec4 vCol;"
"out vec2 vFragUv;"
"out vec4 vFragCol;"

"void main()"
"{"
"   gl_Position = mPixelToNormalized * vec4(vPos, 0.0, 1.0);"
"   vFragUv = vTexelToNormalized * vUv;"
"   vFragCol = vCol;"
"}";

static const char* fragShader_src =
_IMPL_GLSL_VERSION_HEADER
"precision mediump float;"
"in vec2 vFragUv;"
"in vec4 vFragCol;"
"out vec4 vFinalFragCol;"

"uniform sampler2D sTexture;"

"void main() {"
"   vFinalFragCol = texture(sTexture, vFragUv).rgba * vFragCol;"
"}";

RenderGuiGlsl::~RenderGuiGlsl() {
    if (m_glVertexBuffer)
        glDeleteBuffers(1, &m_glVertexBuffer);
    if (m_glIndexBuffer)
        glDeleteBuffers(1, &m_glIndexBuffer);
}

bool RenderGuiGlsl::Init() {
    uint8_t white_px[4] = { 255, 255, 255, 255 };
    m_default_texture = Texture::Create(TextureInfo(TextureFormat::RGBA_8_32, 1, 1), white_px);

    static CShaderGlsl vertShader;
    static CShaderGlsl fragShader;

    if (!vertShader.Compile(GL_VERTEX_SHADER, vertShader_src) ||
        !fragShader.Compile(GL_FRAGMENT_SHADER, fragShader_src))
        return false;
    
    if (!m_glProgram.AttachShader(vertShader) ||
        !m_glProgram.AttachShader(fragShader))
    {
        PLATFORM_ERROR("Failed to attach shaders");
        return false;
    }

    m_glProgram.Link();

    m_mPixelToNormalized = m_glProgram.GetUniformLocation("mPixelToNormalized");
    m_vTexelToNormalized = m_glProgram.GetUniformLocation("vTexelToNormalized");
    m_vPos = m_glProgram.GetAttribLocation("vPos");
    m_vUv = m_glProgram.GetAttribLocation("vUv");
    m_vCol = m_glProgram.GetAttribLocation("vCol");

    glGenBuffers(1, &m_glVertexBuffer);
    glGenBuffers(1, &m_glIndexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
    glEnableVertexAttribArray(m_vPos);
    glEnableVertexAttribArray(m_vUv);
    glEnableVertexAttribArray(m_vCol);
    glVertexAttribPointer(m_vPos, 2, GL_FLOAT, GL_FALSE, sizeof(gui::Vertex), (void*)0);
    glVertexAttribPointer(m_vUv, 2, GL_FLOAT, GL_FALSE, sizeof(gui::Vertex), (void*)(2 * sizeof(float)));
    glVertexAttribPointer(m_vCol, 4, GL_FLOAT, GL_FALSE, sizeof(gui::Vertex), (void*)(4 * sizeof(float)));

    return true;
}

void RenderGuiGlsl::UploadDrawData(const gui::DrawList& list) {
    m_drawlist = &list;
    
    glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIndexBuffer);
    
    glBufferData(GL_ARRAY_BUFFER,
        list.vertices.size() * sizeof(list.vertices[0]),
        list.vertices.data(), GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        list.indices.size() * sizeof(list.indices[0]),
        list.indices.data(), GL_STREAM_DRAW);
}

void RenderGuiGlsl::Render() {
    float width = ScreenWidth();
    float height = ScreenHeight();
    float aspect = width / height;
    
    glViewport(0, 0, width, height);
    glUseProgram(m_glProgram.GlHandle());
    glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIndexBuffer);

    glm::mat4x4 m = glm::mat4x4(1.f);
    m = glm::ortho<float>(0, ScreenWidth(), ScreenHeight(), 0, 1, -1);
    glUniformMatrix4fv(m_mPixelToNormalized, 1, GL_FALSE, (const GLfloat*)&m[0][0]);

    for (const gui::DrawCall& call : m_drawlist->calls) {
        // Bind texture
        Texture::Ptr current_tex = call.texture;
        if (!current_tex)
            current_tex = m_default_texture;
        std::shared_ptr<COpenGLTexture> gl_tex = std::dynamic_pointer_cast<COpenGLTexture>(current_tex);
        glBindTexture(GL_TEXTURE_2D, gl_tex->GetId());
        glUniform2f(m_vTexelToNormalized, 1.f / gl_tex->GetWidth(), 1.f / gl_tex->GetHeight());

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
}
