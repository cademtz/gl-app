#pragma once
#include <platform.hpp>
#include <string>
#include "opengl.hpp"

class OglShader
{
public:
    OglShader(GLenum type, std::string&& glsl_source) : m_type(type), m_glsl_source(std::move(glsl_source)) {}
    ~OglShader() { DeleteGlShader(); }

    bool Compile();

    GLenum GlType() const { return m_type; }
    GLuint GlHandle() const { return m_glShader; }

private:
    void DeleteGlShader();

    bool m_is_ready = false;
    std::string m_glsl_source;
    GLuint m_glShader = 0;
    GLenum m_type = 0;
};