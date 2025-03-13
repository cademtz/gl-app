#include "oglshader.hpp"
#include <array>
#include <vector>
#include <cstddef>
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

OglProgram::OglProgram() {
    m_gl_program = glCreateProgram();
}

OglProgram::~OglProgram() {
    glDeleteProgram(m_gl_program);
}

bool OglProgram::AttachShader(const OglShader& shader) {
    while (glGetError() != GL_NO_ERROR) {} // Clear existing errors
    glAttachShader(m_gl_program, shader.GlHandle());
    return glGetError() == GL_NO_ERROR;
}

bool OglProgram::Link() {
    while (glGetError() != GL_NO_ERROR) {} // Clear existing errors

    glLinkProgram(m_gl_program);
    if (glGetError() != GL_NO_ERROR)
        return false;

    GLint link_status;
    glGetProgramiv(m_gl_program, GL_LINK_STATUS, &link_status);
    if (link_status == GL_FALSE) {
        GLint log_length;
        glGetProgramiv(m_gl_program, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<GLchar> log(log_length);

        GLsizei length;
        glGetProgramInfoLog(m_gl_program, log.size(), &length, log.data());

        std::string_view log_view{log.data(), log.size()};
        PLATFORM_WARNING(log_view);
        return false;
    }

    // Dump all the uniforms into a map.
    // Then we can look them up without weird stutters (on Windows, at least).

    GLint num_uniforms;
    glGetProgramiv(m_gl_program, GL_ACTIVE_UNIFORMS, &num_uniforms);
    for (GLint i = 0; i < num_uniforms; ++i) {
        std::array<GLchar, 64> name;
        GLsizei name_len;
        GLint uniform_size;
        GLenum type;
        glGetActiveUniform(m_gl_program, (GLuint)i, name.size(), &name_len, &uniform_size, &type, name.data());

        GLint location;
        location = glGetUniformLocation(m_gl_program, name.data());

        std::string_view name_view{name.data(), (size_t)name_len};
        m_uniform_map.emplace(std::pair<std::string, GLuint>{name_view, (GLuint)location});
    }
    
    return true;
}

GLint OglProgram::GetUniformLocation(std::string_view name) const {
    auto it = m_uniform_map.find(name);
    if (it == m_uniform_map.end())
        return -1;
    return it->second;
}
GLint OglProgram::GetAttribLocation(std::string_view name) const {
    return glGetAttribLocation(m_gl_program, name.data());
}