#include "renderguiglsl.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static const char* vertShader_src =
"#version 300 es\n"
"uniform mat4 mPixelToNormalized;"
"in vec2 vPos;"
"in vec2 vUv;"
"in vec4 vCol;"

"out vec2 vFragUv;"
"out vec4 vFragCol;"

"void main()"
"{"
"   gl_Position = mPixelToNormalized * vec4(vPos, 0.0, 1.0);"
"   vFragUv = vUv;"
"   vFragCol = vCol;"
"}";

static const char* fragShader_src =
"#version 300 es\n"
"precision mediump float;"
"in vec2 vFragUv;"
"in vec4 vFragCol;"
"out vec4 vFinalFragCol;"

"uniform sampler2D sTexture;"

"void main() {"
"   vFinalFragCol = /*texture(sTexture, vFragUv).rgba * */vFragCol;"
"}";

CRenderGuiGlsl::~CRenderGuiGlsl()
{
    if (m_glVertexBuffer)
        glDeleteBuffers(1, &m_glVertexBuffer);
    if (m_glIndexBuffer)
        glDeleteBuffers(1, &m_glIndexBuffer);
}

bool CRenderGuiGlsl::Init()
{
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
    m_vPos = m_glProgram.GetAttribLocation("vPos");
    m_vUv = m_glProgram.GetAttribLocation("vUv");
    m_vCol = m_glProgram.GetAttribLocation("vCol");

    glGenBuffers(1, &m_glVertexBuffer);
    glGenBuffers(1, &m_glIndexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
    glEnableVertexAttribArray(m_vPos);
    glEnableVertexAttribArray(m_vUv);
    glEnableVertexAttribArray(m_vCol);
    glVertexAttribPointer(m_vPos, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2d), (void*)0);
    glVertexAttribPointer(m_vUv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2d), (void*)(2 * sizeof(float)));
    glVertexAttribPointer(m_vCol, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2d), (void*)(4 * sizeof(float)));

    return true;
}

void CRenderGuiGlsl::UploadGeometry(const Geometry2d& Geometry)
{
    m_uploadedIndices = Geometry.indices.size();
    
    glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIndexBuffer);
    
    glBufferData(GL_ARRAY_BUFFER,
        Geometry.vertices.size() * sizeof(Geometry.vertices[0]),
        Geometry.vertices.data(), GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        Geometry.indices.size() * sizeof(Geometry.indices[0]),
        Geometry.indices.data(), GL_STREAM_DRAW);
}

void CRenderGuiGlsl::Render()
{
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

    glDrawElements(GL_TRIANGLES, m_uploadedIndices, GL_UNSIGNED_INT, nullptr);
}
