#pragma once
#include <impl/render/opengl/opengl.hpp>
#include "shaderglsl.hpp"

class CProgramGlsl
{
public:
    CProgramGlsl();
    ~CProgramGlsl();

    bool AttachShader(const CShaderGlsl& Shader);
    bool Link();

    GLint GetUniformLocation(const char* Name) const;
    GLint GetAttribLocation(const char* Name) const;
    GLint GlHandle() const { return m_glProgram; }

private:
    GLint m_glProgram;
};