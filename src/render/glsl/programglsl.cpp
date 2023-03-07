#include "programglsl.hpp"
#include <vector>

CProgramGlsl::CProgramGlsl() {
    m_glProgram = glCreateProgram();
}

CProgramGlsl::~CProgramGlsl() {
    glDeleteProgram(m_glProgram);
}

bool CProgramGlsl::AttachShader(const CShaderGlsl& Shader)
{
    glAttachShader(m_glProgram, Shader.GlHandle());
    return glGetError() == GL_NO_ERROR;
}

bool CProgramGlsl::Link()
{
    glLinkProgram(m_glProgram);

    GLint linkStatus;
    glGetProgramiv(m_glProgram, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE)
    {
        GLint log_length;
        glGetProgramiv(m_glProgram, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<GLchar> log(log_length);

        GLsizei length;
        glGetProgramInfoLog(m_glProgram, log.size(), &length, log.data());

        PLATFORM_ERROR(log.data());
        return false;
    }
    
    return true;
}

GLint CProgramGlsl::GetUniformLocation(const char* Name) const {
    return glGetUniformLocation(m_glProgram, Name);
}
GLint CProgramGlsl::GetAttribLocation(const char* Name) const {
    return glGetAttribLocation(m_glProgram, Name);
}