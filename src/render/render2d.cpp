#include "render2d.hpp"
#include "texture.hpp"
#include "opengl/oglshader.hpp"
#include "opengl/oglframebuffer.hpp"
#include <platform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Compile these files together
#include "render2d_draw.cpp"

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

namespace Render2d {

OglShaderPtr GetDefaultVertShader() {
    static OglShaderPtr obj = OglShader::Compile(ShaderType::VERTEX, VERT_SHADER_SRC);
    if (obj == nullptr)
        PLATFORM_ERROR("Failed to compile default vertex shader");
    return obj;
}
OglShaderPtr GetDefaultFragShader() {
    static OglShaderPtr obj = OglShader::Compile(ShaderType::FRAGMENT, FRAG_SHADER_SRC);
    if (obj == nullptr)
        PLATFORM_ERROR("Failed to compile default fragment shader");
    return obj;
}
OglProgramPtr GetDefaultProgram() {
    static OglProgramPtr program;
    if (program == nullptr) {
        OglProgramPtr new_program = std::make_shared<OglProgram>();
        if (!new_program->AttachShader(*GetDefaultVertShader())
            || !new_program->AttachShader(*GetDefaultFragShader())
            || !new_program->Link()
        ) {
            PLATFORM_ERROR("Failed to link default shaders");
            return nullptr;
        }
        program = new_program;
    }
    return program;
}
TexturePtr GetDefaultTexture() {
    uint8_t white_px[4] = { 255, 255, 255, 255 };
    static TexturePtr t = Texture::Create(TextureInfo(TextureFormat::RGBA_8_32, 1, 1), white_px);
    return t;
}

static OglFramebuffer& GetFrameBuffer() {
    static OglFramebuffer buffer;
    return buffer;
}

static const DrawList* m_drawlist;
static GLuint m_vertex_buffer;
static GLuint m_index_buffer;
static GLuint m_array_object;

bool Setup() {
    glGenBuffers(1, &m_vertex_buffer);
    glGenBuffers(1, &m_index_buffer);
    glGenVertexArrays(1, &m_array_object);

    return true;
}

void Cleanup() {
    if (m_vertex_buffer)
        glDeleteBuffers(1, &m_vertex_buffer);
    if (m_index_buffer)
        glDeleteBuffers(1, &m_index_buffer);
    if (m_array_object)
        glDeleteVertexArrays(1, &m_array_object);
}

void UploadDrawData(const DrawList& list) {
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

void Render() {
    float aspect = m_screen_w / m_screen_h;

    glm::mat4x4 m;

    if (render_target) {
        render_target->SetPremultiplied(true);
        GetFrameBuffer().SetColorAttachment(render_target);
        glBindFramebuffer(GL_FRAMEBUFFER, GetFrameBuffer().GlHandle());
        m = glm::ortho<float>(0, m_screen_w, 0, m_screen_h, 1, -1);
    } else {
        m = glm::ortho<float>(0, m_screen_w, m_screen_h, 0, 1, -1);
    }


    glViewport(0, 0, m_screen_w, m_screen_h);
    glBindVertexArray(m_array_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);

    for (const DrawCall& call : m_drawlist->calls) {
        // Bind program
        OglProgramPtr program = call.program;
        if (program == nullptr)
            program = GetDefaultProgram();
        glUseProgram(program->GlHandle());

        GLint pixel_to_normalized = program->GetUniformLocation("pixel_to_normalized");
        GLint texel_to_normalized = program->GetUniformLocation("texel_to_normalized");
        glUniformMatrix4fv(pixel_to_normalized, 1, GL_FALSE, (const GLfloat*)&m[0][0]);

        // Bind texture
        TexturePtr current_tex = call.texture;
        if (!current_tex)
            current_tex = GetDefaultTexture();
        
        if (current_tex->GetInfo().premul)
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        else if (render_target)
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
        else 
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, current_tex->GlHandle());
        glUniform2f(texel_to_normalized, 1.f / current_tex->GetInfo().width, 1.f / current_tex->GetInfo().height);

        GLint attrib_in_pos = program->GetAttribLocation("in_pos");
        GLint attrib_in_uv = program->GetAttribLocation("in_uv");
        GLint attrib_in_color = program->GetAttribLocation("in_color");
        
        // Bind array attributes
        if (attrib_in_pos != -1) {
            glEnableVertexAttribArray(attrib_in_pos);
            glVertexAttribPointer(attrib_in_pos,   2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));
        }
        if (attrib_in_uv != -1) {
            glEnableVertexAttribArray(attrib_in_uv);
            glVertexAttribPointer(attrib_in_uv,    2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(2 * sizeof(float)));
        }
        if (attrib_in_color != -1) {
            glEnableVertexAttribArray(attrib_in_color);
            glVertexAttribPointer(attrib_in_color, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(4 * sizeof(float)));
        }

        // Clip geometry
        if (glm::isnan(call.clip_rect.x))
            glDisable(GL_SCISSOR_TEST);
        else {
            glEnable(GL_SCISSOR_TEST);
            // glScissor uses y=0 as the bottom of the screen
            
            glm::vec<4, int32_t> irect;
            for (int i = 0; i < 4; ++i)
                irect[i] = (int32_t)glm::round(call.clip_rect[i]);

            float new_y = irect.y;
            if (!render_target)
                new_y = m_screen_h - irect.y - irect[3];
            glScissor(irect.x, new_y, irect[2], irect[3]);
        }

        glDrawElements(GL_TRIANGLES, call.index_count, GL_UNSIGNED_INT, (void*)(call.index_offset * sizeof(GLuint)));
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_SCISSOR_TEST);
    glBindVertexArray(0);
    glUseProgram(0);

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        printf("glError: %X\n", err);
    }
}

}