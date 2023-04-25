#include "texture.hpp"
#include <cassert>

uint32_t CTextureInfo::GetPixelStride() const {
    switch (GetFormat()) {
    case TextureFormat::A_8_8: return 1;
    case TextureFormat::RGB_8_24: return 3;
    case TextureFormat::RGBA_8_32: return 4;
    default:
        assert(0 && "Missing texture format case!");
    }
    return ~(uint32_t)0;
}

CClientTexture::Ptr CClientTexture::Convert(TextureFormat new_format, ExpandOp operation)
{
    CClientTexture::Ptr new_tex = CClientTexture::Create(CTextureInfo(new_format, GetWidth(), GetHeight()));
    std::array<uint8_t, 4> next_pixel = { 0 };
    for (uint8_t y = 0; y < GetHeight(); ++y) {
        for (uint8_t x = 0; x < GetWidth(); ++x) {
            // Read channels from current pixel
            uint8_t* input_ptr = GetPixel(x, y);
            for (uint8_t i = 0; i < GetPixelStride(); ++i)
                next_pixel[i] = input_ptr[i];
                
            // Send a copy of next_pixel and its reference to be modified 
            operation(next_pixel, next_pixel);
            // Output the now-modified channels to the new texture
            uint8_t* output_ptr = new_tex->GetPixel(x, y);
            for (uint8_t i = 0; i < new_tex->GetPixelStride(); ++i)
                output_ptr[i] = next_pixel[i];
        }
    }
    return new_tex;
}

CClientTexture::Ptr CClientTexture::Create(CTextureInfo &&info)
{
    uint8_t* data = new uint8_t[info.GetRowStride() * info.GetHeight()];
    return std::make_shared<CClientTexture>(CClientTexture(std::move(info), data));
}