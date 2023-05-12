#include "texture.hpp"
#include "render/texture.hpp"

Texture::Ptr Texture::Create(TextureInfo&& info, const void* data) {
    int gl_format;
    switch (info.GetFormat()) {
    case TextureFormat::RGB_8_24: gl_format = GL_RGB; break;
    case TextureFormat::RGBA_8_32: gl_format = GL_RGBA; break;
    case TextureFormat::A_8_8: gl_format = GL_ALPHA; break;
    default:
        assert(0 && "Texture format not implemented");
    }

    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    //// Credit: IMGUI docs (https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples#example-for-opengl-users)
    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    ////

    glTexImage2D(GL_TEXTURE_2D, 0, gl_format, info.GetWidth(), info.GetHeight(), 0, gl_format, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0); // Bind default texture to catch errors in future calls

    return std::make_shared<COpenGLTexture>(std::move(info), id);
}