#pragma once

#include <memory>
#include <cstdint>

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

class CTextureInfo {
public:
    CTextureInfo(TextureFormat format, uint32_t width, uint32_t height)
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
class CClientTexture : public CTextureInfo {
public:
    using Ptr = std::shared_ptr<CClientTexture>;

    ~CClientTexture() {
        delete[] m_data;
    }

    const uint8_t* GetData() const { return m_data; }
    uint8_t* GetData() { return m_data; }

    static Ptr Create(CTextureInfo&& info);

protected:
    CClientTexture(CTextureInfo&& info, uint8_t* data) : CTextureInfo(info), m_data(data) {}
    
private:
    uint8_t* const m_data;
};

/**
 * @brief Handle to a texture on the rendering backend.
 * The texture data may be stored on a GPU, and thus cannot be directly read or written.
 */
class CTexture : public CTextureInfo
{
public:
    using Ptr = std::shared_ptr<CTexture>;

    virtual ~CTexture() {}

    /**
     * @brief Write new data to a specified portion of the texture
     */
    virtual void Write(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data) = 0;

    /**
     * @brief Create a new texture. (implementation-defined)
     * @param data Initial data for the texture. If `data = nullptr`, the initial pixels are undefined
     */
    static Ptr Create(CTextureInfo&& info, const void* data = nullptr);

    static Ptr Create(CClientTexture::Ptr texture) {
        return Create(CTextureInfo(*texture), texture->GetData());
    }

protected:
    CTexture(CTextureInfo&& info) : CTextureInfo(info) {}
};