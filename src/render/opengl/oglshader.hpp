#pragma once
#include "opengl.hpp"
#include "forward.hpp"
#include <platform.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

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

    void SetInt(GLint uniform, int val);
    void SetFloat(GLint uniform, float val);
    void SetVec2(GLint uniform, const glm::vec2& val);
    void SetVec3(GLint uniform, const glm::vec3& val);
    void SetVec4(GLint uniform, const glm::vec4& val);
    void SetMat3(GLint uniform, const glm::mat3& val);
    void SetMat4(GLint uniform, const glm::mat4& val);

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