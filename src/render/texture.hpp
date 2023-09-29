#pragma once
#include <array>
#include <memory>
#include <cstdint>
#include <functional>

enum class TextureFormat : char {
    /** RGB, 8-bit channels, total of 24 bits */
    RGB_8_24,
    /** RGBA, 8-bit channels, total of 32 bits */
    RGBA_8_32,
    /** Alpha, 8-bit channel */
    A_8_8,
};

/** Indicate the image format to use */
enum class ImageTypeHint {
    NONE, PNG, JPG, GIF, BMP
};

class TextureInfo {
public:
    TextureInfo(TextureFormat format, uint32_t width, uint32_t height)
        : m_format(format), m_width(width), m_height(height) {}

    TextureFormat GetFormat() const { return m_format; }
    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }
    /** @return Number of bytes per pixel */
    uint32_t GetPixelStride() const;
    /** @return Number of bytes per row */
    uint32_t GetRowStride() const { return GetPixelStride() * GetWidth(); }

protected:
    TextureFormat m_format;
    uint32_t m_width;
    uint32_t m_height;
};

/**
 * @brief Texture data on the client
 */
class ClientTexture : public TextureInfo {
public:
    using Ptr = std::shared_ptr<ClientTexture>;
    using ConstPtr = std::shared_ptr<const ClientTexture>;
    using ExpandOp = std::function<void(std::array<uint8_t, 4> input, std::array<uint8_t, 4>& output)>;

    ClientTexture(ClientTexture&& other)
        : TextureInfo(other), m_data(other.m_data), m_free(other.m_free) {
        other.m_width = 0, other.m_height = 0;
        other.m_data = nullptr;
    }
    ~ClientTexture() { m_free(m_data); }

    const uint8_t* GetData() const { return m_data; }
    uint8_t* GetData() { return m_data; }
    const uint32_t GetPixelOffset(uint32_t x, uint32_t y) const {
        return y * GetRowStride() + x * GetPixelStride();
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
    bool Write(ClientTexture::ConstPtr new_data, uint32_t x, uint32_t y, uint32_t w = ~(uint32_t)0, uint32_t h = ~(uint32_t)0);
    /** Create an new texture by converting each pixel's channels to a new format */
    Ptr Convert(TextureFormat new_format, ExpandOp operation);
    static Ptr Create(const TextureInfo& info);
    /**
     * @brief Attempt to parse an image file
     * @param type_hint The image format to parse. May be @ref ImageTypeHint::NONE
     * @param img_data Pointer to image data
     * @param img_size Size of image data, in bytes
     * @return A new texture, or `nullptr`
     */
    static Ptr FromImage(ImageTypeHint type_hint, const void* img_data, size_t img_size);

protected:
    using FreeFn = void(void* data);

    /**
     * @param free Required. When freeing a `nullptr`, no action shall occur.
     */
    ClientTexture(const TextureInfo& info, uint8_t* data, FreeFn free)
        : TextureInfo(info), m_data(data), m_free(free) {}
    ClientTexture(const ClientTexture&) = delete;
    
private:
    uint8_t* m_data;
    FreeFn* const m_free;
};

/**
 * @brief Handle to a texture on the rendering backend.
 * The texture data may be stored on a GPU, and thus cannot be directly read or written.
 */
class Texture : public TextureInfo
{
public:
    using Ptr = std::shared_ptr<Texture>;

    /** Resize the texture. Contents will become undefined. */
    virtual void Resize(uint32_t width, uint32_t height) = 0;
    /** Write new data to a specified portion of the texture */
    virtual void Write(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data) = 0;
    /** Set all pixels to one color */
    virtual void ClearColor(float r, float g, float b, float a) = 0;
    /**
     * @brief Create a new texture. (implementation-defined)
     * @param data Initial data for the texture. If `data == nullptr`, the initial pixels are undefined
     */
    static Ptr Create(TextureInfo&& info, const void* data = nullptr);
    static Ptr Create(ClientTexture::Ptr texture) {
        return Create(TextureInfo(*texture), texture->GetData());
    }

protected:
    Texture(TextureInfo&& info) : TextureInfo(std::move(info)) {}
};