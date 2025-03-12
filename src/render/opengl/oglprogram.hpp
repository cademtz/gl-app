#pragma once
#include "opengl.hpp"
#include <unordered_map>
#include <string_view>

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