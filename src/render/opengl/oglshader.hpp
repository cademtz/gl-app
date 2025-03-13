#pragma once
#include "opengl.hpp"
#include "forward.hpp"
#include <platform.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

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

class OglShader;

class OglProgram
{
public:
    OglProgram();
    ~OglProgram();

    bool AttachShader(const OglShader& shader);
    bool Link();

    GLint GetUniformLocation(std::string_view name) const;
    GLint GetAttribLocation(std::string_view name) const;
    GLint GlHandle() const { return m_gl_program; }

private:
    struct StringHash {
        using is_transparent = void;
        size_t operator()(const char* str) const {
            return std::hash<std::string_view>{}(str);
        }
        size_t operator()(std::string_view str) const {
            return std::hash<std::string_view>{}(str);
        }
        size_t operator()(const std::string& str) const {
            return std::hash<std::string>{}(str);
        }
    };

    OglProgram(const OglProgram&) = delete;

    GLint m_gl_program;
    std::unordered_map<std::string, GLuint, StringHash, std::equal_to<>> m_uniform_map;
};