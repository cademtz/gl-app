#pragma once
#include <stdint.h>
#include <algorithm>
#include <vector>
#include <memory>
#include <stb_rect_pack.h>

typedef uint32_t rgba_t;

class CTexture
{
public:
    CTexture(uint32_t Width, uint32_t Height) : m_width(Width), m_height(Height) { }
    virtual ~CTexture() = 0;

    virtual void WriteRow(const rgba_t* Pixels, uint32_t PixelCount) = 0;

    uint32_t Width() const { return m_width; }
    uint32_t Height() const { return m_height; }

private:
    const uint32_t m_width, m_height;
};

class CTextureStream
{
public:
    CTextureStream(uint32_t Width, uint32_t Height) : m_width(Width), m_height(Height) { }
    virtual ~CTextureStream() = 0;

    virtual bool ReadNextRow(rgba_t* PixelBuffer, uint32_t PixelBufferCount) = 0;
    virtual bool ResetStream() = 0;

private:
    const uint32_t m_width, m_height;    
};

struct TexturePackNode
{
    std::shared_ptr<CTextureStream> input;
    stbrp_rect& output;
};

// Allocate texture atlas and pack textures into it
class CTexturePack
{
public:
    // Creates a square atlas
    CTexturePack(uint32_t MaxBytes);
    CTexturePack(uint32_t Width, uint32_t Height);
    virtual ~CTexturePack() = 0;

    virtual std::shared_ptr<CTexture> CreateTexture(uint32_t Width, uint32_t Height) = 0;

    template <class T>
    std::shared_ptr<CTexture> GenerateAtlas(const T& Textures)
    {
        // Allocate atlas
        
    }

    template <class T>
    static uint32_t GetMaxWidth(const T& Textures)
    {
        return std::max_element(Textures.cbegin(), Textures.cend(),
            [](const CTextureStream* left, const CTextureStream* right) {
                return left.Width() > right.Width();
            });
    }

    template <class T>
    static uint32_t GetMaxHeight(const T& Textures)
    {
        return std::max_element(Textures.cbegin(), Textures.cend(),
            [](const CTextureStream* left, const CTextureStream* right) {
                return left.Height() > right.Height();
            });
    }

private:
    template <class T>
    bool PackTextures();
};