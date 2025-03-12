#include "oglprogram.hpp"
#include "oglshader.hpp"
#include <vector>
#include <array>

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