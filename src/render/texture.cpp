#include "texture.hpp"
#include "opengl/oglframebuffer.hpp"
#include <platform.hpp>
#include <algorithm>
#include <cstring> // memcpy

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
#define STBI_ONLY_GIF
#include <stb_image.h>

static void CppFreeArray(void* data) { delete[] (uint8_t*)data; }
static void StbiFree(void* data) { STBI_FREE(data); }

uint32_t TextureInfo::GetPixelStride() const {
    switch (format) {
    case TextureFormat::A_8_8: return 1;
    case TextureFormat::RGB_8_24: return 3;
    case TextureFormat::RGBA_8_32: return 4;
    default:
        assert(0 && "Missing texture format case!");
    }
    return ~(uint32_t)0;
}

bool ClientTexture::Write(ClientTextureConstPtr new_data, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    // Mismatched formats, or texture coords are completely out of bounds
    if (m_info.format != new_data->m_info.format || x >= m_info.width || y >= m_info.height)
        return false;
    
    if (w == ~(uint32_t)0)
        w = new_data->m_info.width;
    if (h == ~(uint32_t)0)
        h = new_data->m_info.height;
    
    assert(w <= new_data->m_info.width && h <= new_data->m_info.height);

    uint32_t write_w = w;
    uint32_t write_h = h;
    if (x + write_w > m_info.width)
        write_w = m_info.width - x;
    if (y + write_h > m_info.height)
        write_h = m_info.height - y;

    for (uint32_t offset_y = write_h - 1; offset_y < ~(~(uint32_t)0 >> 1); --offset_y) {
        const uint8_t* src_row = new_data->GetPixel(0, offset_y);
        uint8_t* dst_row = GetPixel(x, y + offset_y);
        std::memcpy(dst_row, src_row, m_info.GetPixelStride() * write_w);
    }
    return true;
}

ClientTexturePtr ClientTexture::Convert(TextureFormat new_format, ExpandOp operation) {
    ClientTexturePtr new_tex = ClientTexture::Create(TextureInfo(new_format, m_info.width, m_info.height));
    std::array<uint8_t, 4> next_pixel = { 0 };
    for (uint32_t y = 0; y < m_info.height; ++y) {
        for (uint32_t x = 0; x < m_info.width; ++x) {
            // Read channels from current pixel
            uint8_t* input_ptr = GetPixel(x, y);
            for (uint8_t i = 0; i < m_info.GetPixelStride(); ++i)
                next_pixel[i] = input_ptr[i];
                
            // Send a copy of next_pixel and its reference to be modified 
            operation(next_pixel, next_pixel);
            // Output the now-modified channels to the new texture
            uint8_t* output_ptr = new_tex->GetPixel(x, y);
            for (uint8_t i = 0; i < new_tex->m_info.GetPixelStride(); ++i)
                output_ptr[i] = next_pixel[i];
        }
    }
    return new_tex;
}

ClientTexturePtr ClientTexture::Create(const TextureInfo &info) {
    uint8_t* data = new uint8_t[info.GetRowStride() * info.height];
    return std::make_shared<ClientTexture>(ClientTexture(info, data, &CppFreeArray));
}

ClientTexturePtr ClientTexture::FromImage(ImageTypeHint type_hint, const void* img_data, size_t img_size) {
    if (img_size > INT_MAX)
        return nullptr;
    
    int width, height, comp;
    if (!stbi_info_from_memory((stbi_uc*)img_data, (int)img_size, &width, &height, &comp))
        return nullptr;
    
    TextureFormat fmt;
    int req_comp;
    switch (comp) {
    case STBI_grey:
        fmt = TextureFormat::A_8_8;
        req_comp = STBI_grey;
        break;
    case STBI_grey_alpha:
        // No support for gray + alpha, yet. We'll just use extra space for now. 
        fmt = TextureFormat::RGBA_8_32;
        req_comp = STBI_rgb_alpha;
        break;
    case STBI_rgb:
        fmt = TextureFormat::RGB_8_24;
        req_comp = STBI_rgb;
        break;
    case STBI_rgb_alpha:
        fmt = TextureFormat::RGBA_8_32;
        req_comp = STBI_rgb_alpha;
        break;
    default:
        return nullptr; // Unsupported texture format
    }
    
    stbi_uc* pixels = stbi_load_from_memory((stbi_uc*)img_data, (int)img_size, &width, &height, &comp, req_comp);
    if (!pixels)
        return nullptr;
    
    return std::make_shared<ClientTexture>(ClientTexture({fmt, (uint32_t)width, (uint32_t)height}, (uint8_t*)pixels, &StbiFree));
}

static int GetGlFormat(TextureFormat fmt) {
    switch (fmt) {
    case TextureFormat::RGB_8_24: return GL_RGB;
    case TextureFormat::RGBA_8_32: return GL_RGBA;
    case TextureFormat::A_8_8: return GL_ALPHA;
    default:
        PLATFORM_ERROR("Texture format not implemented");
        return 0;
    }
}

static OglFramebuffer& GetUtilFramebuffer() {
    static OglFramebuffer framebuf;
    return framebuf;
}

void Texture::Resize(uint32_t width, uint32_t height) {
    int gl_format = GetGlFormat(m_info.format);
    glBindTexture(GL_TEXTURE_2D, GlHandle());
    glTexImage2D(GL_TEXTURE_2D, 0, gl_format, width, height, 0, gl_format, GL_UNSIGNED_BYTE, nullptr);

    m_info.width = width;
    m_info.height = height;
}

void Texture::Write(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data) {
    glBindTexture(GL_TEXTURE_2D, GlHandle());
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GetGlFormat(m_info.format), GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::ClearColor(float r, float g, float b, float a) {
    OglFramebuffer& buf = GetUtilFramebuffer();
    buf.SetColorAttachmentInternal(GlHandle(), 0);
    glBindFramebuffer(GL_FRAMEBUFFER, buf.GlHandle());
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    buf.SetColorAttachmentInternal(0, 0);
}

TexturePtr Texture::Create(const TextureInfo& info, const void* data) {
    while (glGetError() != GL_NO_ERROR) {};
    int gl_format = GetGlFormat(info.format);

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

    glTexImage2D(GL_TEXTURE_2D, 0, gl_format, info.width, info.height, 0, gl_format, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0); // Bind default texture to catch errors in future calls

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        PLATFORM_WARNING("Failed to create texture");
        return nullptr;
    }

    return std::make_shared<Texture>(info, id);
}