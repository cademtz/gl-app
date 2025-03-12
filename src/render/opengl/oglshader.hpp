#pragma once
#include <platform.hpp>
#include <string>
#include "opengl.hpp"
#include "forward.hpp"

enum class ShaderType {
    UNKNOWN, VERTEX, FRAGMENT,
};

class OglShader
{
public:
    OglShader(GLenum type, GLuint shader_handle) : m_type(type), m_gl_shader(shader_handle) {}
    ~OglShader();

    /**
     * @param file_ext (optional) File extension without a period (such as "vert")
     */
    static OglShaderPtr Compile(ShaderType type, std::string_view source, std::string_view file_ext = {});

    GLenum GlType() const { return m_type; }
    GLuint GlHandle() const { return m_gl_shader; }

private:
    OglShader(const OglShader&) = delete;

    GLuint m_gl_shader = 0;
    GLenum m_type = 0;
};