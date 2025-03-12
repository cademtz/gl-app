#include "ogltexture.hpp"
#include "oglframebuffer.hpp"

static int GetGlFormat(TextureFormat fmt) {
    switch (fmt) {
    case TextureFormat::RGB_8_24: return GL_RGB;
    case TextureFormat::RGBA_8_32: return GL_RGBA;
    case TextureFormat::A_8_8: return GL_ALPHA;
    default:
        assert(0 && "Texture format not implemented");
        return 0;
    }
}

static OglFramebuffer* GetUtilFramebuffer() {
    static OglFramebuffer framebuf;
    return &framebuf;
}

void OglTexture::Resize(uint32_t width, uint32_t height) {
    int gl_format = GetGlFormat(GetFormat());
    glBindTexture(GL_TEXTURE_2D, GlHandle());
    glTexImage2D(GL_TEXTURE_2D, 0, gl_format, width, height, 0, gl_format, GL_UNSIGNED_BYTE, nullptr);

    m_width = width;
    m_height = height;
}

void OglTexture::Write(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data) {
    glBindTexture(GL_TEXTURE_2D, GlHandle());
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GetGlFormat(GetFormat()), GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void OglTexture::ClearColor(float r, float g, float b, float a) {
    OglFramebuffer* buf = GetUtilFramebuffer();
    buf->SetColorAttachmentInternal(GlHandle(), 0);
    glBindFramebuffer(GL_FRAMEBUFFER, buf->GlHandle());
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    buf->SetColorAttachmentInternal(0, 0);
}

Texture::Ptr Texture::Create(TextureInfo&& info, const void* data) {
    int gl_format = GetGlFormat(info.GetFormat());

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

    assert(glGetError() == GL_NO_ERROR);

    return std::make_shared<OglTexture>(std::move(info), id);
}