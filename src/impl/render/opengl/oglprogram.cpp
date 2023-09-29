#include "oglprogram.hpp"
#include <vector>

OglProgram::OglProgram() {
    m_glProgram = glCreateProgram();
}

OglProgram::~OglProgram() {
    glDeleteProgram(m_glProgram);
}

bool OglProgram::AttachShader(const OglShader& Shader) {
    glAttachShader(m_glProgram, Shader.GlHandle());
    return glGetError() == GL_NO_ERROR;
}

bool OglProgram::Link() {
    glLinkProgram(m_glProgram);

    GLint linkStatus;
    glGetProgramiv(m_glProgram, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE) {
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

GLint OglProgram::GetUniformLocation(const char* Name) const {
    return glGetUniformLocation(m_glProgram, Name);
}
GLint OglProgram::GetAttribLocation(const char* Name) const {
    return glGetAttribLocation(m_glProgram, Name);
}