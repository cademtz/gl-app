#include <memory>
#include <render/sticks/rendersticks.hpp>
#include <render/sticks/drawlist.hpp>
#include "glm/ext/matrix_float4x4.hpp"
#include "impl/render/opengl/shaderglsl.hpp"
#include "programglsl.hpp"
#include "opengl.hpp"

static const char* VERT_SHADER_SRC =
_IMPL_GLSL_VERSION_HEADER
"uniform mat4 transform;"
"in vec2 in_pos;"
"in vec2 in_uv;"
"in vec4 in_color;"
"out vec2 frag_uv;"
"out vec4 frag_color;"

"void main() {"
"   gl_Position = transform * vec4(in_pos, 0.0, 1.0);"
"   frag_uv = in_uv;"
"   frag_color = in_color;"
"}";

static const char* FRAG_SHADER_SRC =
_IMPL_GLSL_VERSION_HEADER
"precision mediump float;"
"in vec2 frag_uv;"
"in vec4 frag_color;"
"out vec4 final_frag_color;"

"void main() {"
"   float dist = frag_uv[0] * frag_uv[0] - frag_uv[1];"
"   float coverage = 0.0;"
"   if (dist < 0.0)"
"       coverage = 1.0;"
"   final_frag_color = vec4(frag_color);"
"   final_frag_color[3] *= coverage;"
"}";

namespace sticks {

class RenderSticksGlsl : public RenderSticks {
public:
    RenderSticksGlsl() : RenderSticks("RenderSticksGlsl") {
        if (!Init())
            PLATFORM_ERROR("Failed to initialize RenderSticksGlsl");
    }
    ~RenderSticksGlsl() {
        if (m_vertex_buffer)
            glDeleteBuffers(1, &m_vertex_buffer);
        if (m_index_buffer)
            glDeleteBuffers(1, &m_index_buffer);
    }

    void UploadDrawData(const DrawList& list) override {
        m_drawlist = &list;
        
        glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);

        glBufferData(GL_ARRAY_BUFFER,
            list.vertices.size() * sizeof(list.vertices[0]),
            list.vertices.data(), GL_STREAM_DRAW
        );
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            list.indices.size() * sizeof(list.indices[0]),
            list.indices.data(), GL_STREAM_DRAW
        );
    }

    void Render() override {
        glUseProgram(m_program.GlHandle());

        glm::mat4x4 m = glm::mat4x4(1.f);
        glUniformMatrix4fv(m_transform, 1, GL_FALSE, (const GLfloat*)&m[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);

        for (const DrawCall& call : m_drawlist->calls) {
            glDrawElements(GL_TRIANGLES, call.index_count, GL_UNSIGNED_INT, (void*)(call.index_offset * sizeof(GLuint)));
        }
    }

private:
    bool Init() {
        static CShaderGlsl vert_shader(GL_VERTEX_SHADER, VERT_SHADER_SRC);
        static CShaderGlsl frag_shader(GL_FRAGMENT_SHADER, FRAG_SHADER_SRC);

        if (!vert_shader.Compile() ||
            !frag_shader.Compile()
        ) {
            return false;
        }
        
        if (!m_program.AttachShader(vert_shader) ||
            !m_program.AttachShader(frag_shader)
        ) {
            return false;
        }

        if (!m_program.Link()) {
            return false;
        }

        m_transform = m_program.GetUniformLocation("transform");
        m_in_pos = m_program.GetAttribLocation("in_pos");
        m_in_uv = m_program.GetAttribLocation("in_uv");
        m_in_color = m_program.GetAttribLocation("in_color");

        glGenBuffers(1, &m_vertex_buffer);
        glGenBuffers(1, &m_index_buffer);

        glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
        glEnableVertexAttribArray(m_in_pos);
        glEnableVertexAttribArray(m_in_uv);
        glEnableVertexAttribArray(m_in_color);
        glVertexAttribPointer(m_in_pos, 2, GL_FLOAT, GL_FALSE, sizeof(sticks::Vertex), (void*)0);
        glVertexAttribPointer(m_in_uv, 2, GL_FLOAT, GL_FALSE, sizeof(sticks::Vertex), (void*)(2 * sizeof(float)));
        glVertexAttribPointer(m_in_color, 4, GL_FLOAT, GL_FALSE, sizeof(sticks::Vertex), (void*)(4 * sizeof(float)));

        return true;
    }

    CProgramGlsl m_program;

    const DrawList* m_drawlist;
    
    GLuint m_transform;
    GLuint m_in_pos, m_in_uv, m_in_color;
    GLuint m_vertex_buffer;
    GLuint m_index_buffer;
};

std::shared_ptr<RenderSticks> RenderSticks::GetInstance() {
    static auto ptr = std::make_shared<RenderSticksGlsl>();
    return ptr;
}

}