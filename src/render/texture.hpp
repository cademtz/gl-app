#pragma once
#include <array>
#include <memory>
#include <cstdint>
#include <functional>
#include "forward.hpp"
#include "opengl/opengl.hpp"

struct TextureInfo {
    /** @param premul Whether the colors will be premultiplied */
    TextureInfo(TextureFormat format, uint32_t width, uint32_t height, bool premul = false)
        : format(format), width(width), height(height), premul(premul) {}

    /** @return Number of bytes per pixel */
    uint32_t GetPixelStride() const;
    /** @return Number of bytes per row */
    uint32_t GetRowStride() const { return GetPixelStride() * width; }

    TextureFormat format;
    uint32_t width;
    uint32_t height;
    /// @brief True if color is premultiplied
    bool premul;
};

/**
 * @brief Texture data on the client
 */
class ClientTexture {
public:
    using ExpandOp = std::function<void(std::array<uint8_t, 4> input, std::array<uint8_t, 4>& output)>;

    ClientTexture(ClientTexture&& other)
        : m_info(other.m_info), m_data(other.m_data), m_free(other.m_free) {
        other.m_info.width = 0, other.m_info.height = 0;
        other.m_data = nullptr;
    }
    ~ClientTexture() { m_free(m_data); }

    const TextureInfo& GetInfo() const { return m_info; }
    const uint8_t* GetData() const { return m_data; }
    uint8_t* GetData() { return m_data; }
    const uint32_t GetPixelOffset(uint32_t x, uint32_t y) const {
        return y * m_info.GetRowStride() + x * m_info.GetPixelStride();
    }
    const uint8_t* GetPixel(uint32_t x, uint32_t y) const {
        return m_data + GetPixelOffset(x, y);
    }
    uint8_t* GetPixel(uint32_t x, uint32_t y) {
        return m_data + GetPixelOffset(x, y);
    }
    
    /**
     * @brief Write new data to a specified location in the texture
     * @return `true` if the new data was compatible and written
     */
    bool Write(ClientTextureConstPtr new_data, uint32_t x, uint32_t y, uint32_t w = ~(uint32_t)0, uint32_t h = ~(uint32_t)0);
    /** Create an new texture by converting each pixel's channels to a new format */
    ClientTexturePtr Convert(TextureFormat new_format, ExpandOp operation);
    static ClientTexturePtr Create(const TextureInfo& info);
    /**
     * @brief Attempt to parse an image file
     * @param type_hint The image format to parse. May be @ref ImageTypeHint::NONE
     * @param img_data Pointer to image data
     * @param img_size Size of image data, in bytes
     * @return A new texture, or `nullptr`
     */
    static ClientTexturePtr FromImage(ImageTypeHint type_hint, const void* img_data, size_t img_size);

protected:
    using FreeFn = void(void* data);

    /**
     * @param free Required. When freeing a `nullptr`, no action shall occur.
     */
    ClientTexture(const TextureInfo& info, uint8_t* data, FreeFn free)
        : m_info(info), m_data(data), m_free(free) {}
    ClientTexture(const ClientTexture&) = delete;
    
private:
    TextureInfo m_info;
    uint8_t* m_data;
    FreeFn* const m_free;
};

/**
 * @brief Handle to a texture on the rendering hardware.
 */
class Texture
{
public:
    Texture(const TextureInfo& info, GLuint id)
        : m_info(info), m_handle(id) {}
    ~Texture() { glDeleteTextures(1, &m_handle); }

    const TextureInfo& GetInfo() const { return m_info; }
    void SetPremultiplied(bool value) { m_info.premul = value; }

    /** Resize the texture. Contents will become undefined. */
    void Resize(uint32_t width, uint32_t height);
    /** Write new data to a specified portion of the texture */
    void Write(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data);
    /** Set all pixels to one color */
    void ClearColor(float r, float g, float b, float a);
    GLuint GlHandle() const { return m_handle; }

    /**
     * @param data Initial data for the texture. If `data == nullptr` then the initial pixels are undefined
     */
    static TexturePtr Create(const TextureInfo& info, const void* data);
    static TexturePtr Create(ClientTexturePtr texture) {
        return Create(texture->GetInfo(), texture->GetData());
    }

private:
    const GLuint m_handle;
    TextureInfo m_info;
};