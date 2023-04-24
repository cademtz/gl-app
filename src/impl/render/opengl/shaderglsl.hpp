#pragma once
#include <platform.hpp>
#include "opengl.hpp"

class CShaderGlsl
{
public:
    ~CShaderGlsl() { DeleteGlShader(); }

    bool Compile(GLenum GlType, const char* GlslSource);

    GLenum GlType() const { return m_type; }
    GLuint GlHandle() const { return m_glShader; }

private:
    void DeleteGlShader();

    GLuint m_glShader = 0;
    GLenum m_type = 0;
};