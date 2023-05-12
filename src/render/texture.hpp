#pragma once
#include <array>
#include <memory>
#include <cstdint>
#include <functional>

enum class TextureFormat : char {
    /**
     * @brief RGB, 8-bit channels, total of 24 bits
     */
    RGB_8_24,
    /**
     * @brief RGBA, 8-bit channels, total of 32 bits
     */
    RGBA_8_32,
    /**
     * @brief Alpha, 8-bit channel
     */
    A_8_8,
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

private:
    const TextureFormat m_format;
    const uint32_t m_width;
    const uint32_t m_height;
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
        : TextureInfo(other), m_data(other.m_data) {
        other.m_data = nullptr;
    }

    ~ClientTexture() {
        if (m_data)
            delete[] m_data;
    }

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
    
    /**
     * @brief Create an new texture by converting each pixel's channels to a new format
     */
    Ptr Convert(TextureFormat new_format, ExpandOp operation);

    static Ptr Create(TextureInfo&& info);

protected:
    ClientTexture(TextureInfo&& info, uint8_t* data) : TextureInfo(info), m_data(data) {}
    ClientTexture(const ClientTexture&) = delete;
    
private:
    uint8_t* m_data;
};

/**
 * @brief Handle to a texture on the rendering backend.
 * The texture data may be stored on a GPU, and thus cannot be directly read or written.
 */
class Texture : public TextureInfo
{
public:
    using Ptr = std::shared_ptr<Texture>;

    virtual ~Texture() {}

    /**
     * @brief Write new data to a specified portion of the texture
     */
    virtual void Write(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data) = 0;

    /**
     * @brief Create a new texture. (implementation-defined)
     * @param data Initial data for the texture. If `data = nullptr`, the initial pixels are undefined
     */
    static Ptr Create(TextureInfo&& info, const void* data = nullptr);

    static Ptr Create(ClientTexture::Ptr texture) {
        return Create(TextureInfo(*texture), texture->GetData());
    }

protected:
    Texture(TextureInfo&& info) : TextureInfo(info) {}
};