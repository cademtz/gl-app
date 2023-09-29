#pragma once
#include "opengl.hpp"
#include "oglshader.hpp"

class OglProgram
{
public:
    OglProgram();
    ~OglProgram();

    bool AttachShader(const OglShader& Shader);
    bool Link();

    GLint GetUniformLocation(const char* Name) const;
    GLint GetAttribLocation(const char* Name) const;
    GLint GlHandle() const { return m_glProgram; }

private:
    GLint m_glProgram;
};