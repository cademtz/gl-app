#include "shaderglsl.hpp"
#include <vector>
#include <cstddef>
#include <array>

bool CShaderGlsl::Compile() {
    if (m_is_ready)
        return true;
    
    m_glShader = glCreateShader(m_type);
    if (!m_glShader)
        return false;
    
    std::array<const char*, 1> sources = { m_glsl_source.c_str() }; 

    glShaderSource(m_glShader, 1, sources.data(), NULL);
    glCompileShader(m_glShader);
    
    GLint result;
    glGetShaderiv(m_glShader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        GLint log_length;
        glGetShaderiv(m_glShader, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<GLchar> log(log_length);

        GLsizei length;
        glGetShaderInfoLog(m_glShader, log.size(), &length, log.data());

        PLATFORM_ERROR(log.data());
        return false;
    }

    m_is_ready = true;
    return true;
}

void CShaderGlsl::DeleteGlShader() {
    if (m_glShader)
        glDeleteShader(m_glShader);
    m_glShader = 0;
}