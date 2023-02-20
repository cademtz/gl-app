#include "shaderglsl.h"
#include <vector>

bool CShaderGlsl::Compile(GLenum GlType, const char* GlslSource)
{
    DeleteGlShader();
    
    m_glShader = glCreateShader(GlType);
    if (!m_glShader)
        return false;
    
    glShaderSource(m_glShader, 1, &GlslSource, NULL);
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

    return true;
}

void CShaderGlsl::DeleteGlShader()
{
    if (m_glShader)
        glDeleteShader(m_glShader);
    m_glShader = 0;
}