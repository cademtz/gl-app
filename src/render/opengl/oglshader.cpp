#include "oglshader.hpp"
#include <array>
#include <vector>
#include <cstddef>
#include <array>
#include <util/str.hpp>
#include <util/defer.hpp>

OglShaderPtr OglShader::Compile(ShaderType type, std::string_view source, std::string_view file_ext) {
    if (type == ShaderType::UNKNOWN) {
        if (!util::strcmpi<char>(file_ext, "vert"))
            type = ShaderType::VERTEX;
        else if (!util::strcmpi<char>(file_ext, "frag"))
            type = ShaderType::FRAGMENT;
        else
            return nullptr;
    }

    GLuint gl_shader_type = 0;
    switch (type) {
    case ShaderType::VERTEX:   gl_shader_type = GL_VERTEX_SHADER; break;
    case ShaderType::FRAGMENT: gl_shader_type = GL_FRAGMENT_SHADER; break;
    default:
        return nullptr;
    }
    
    GLint gl_handle = glCreateShader(gl_shader_type);
    if (!gl_handle)
        return nullptr;

    OglShaderPtr shader = std::make_shared<OglShader>(gl_shader_type, gl_handle);
    
    std::array<const char*, 1> sources = { source.data() }; 
    std::array<GLint , 1> lengths = { (GLint)source.length() }; 

    glShaderSource(gl_handle, sources.size(), sources.data(), NULL);
    glCompileShader(gl_handle);
    
    GLint result;
    glGetShaderiv(gl_handle, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        GLint log_length;
        glGetShaderiv(gl_handle, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<GLchar> log(log_length);

        GLsizei length;
        glGetShaderInfoLog(gl_handle, log.size(), &length, log.data());

        std::string_view log_view{log.data(), log.size()};
        PLATFORM_WARNING(log_view);
        return nullptr;
    }

    return shader;
}

OglShader::~OglShader() {
    if (m_gl_shader)
        glDeleteShader(m_gl_shader);
    m_gl_shader = 0;
}