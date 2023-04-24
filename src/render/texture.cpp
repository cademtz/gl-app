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

CClientTexture::Ptr CClientTexture::Create(CTextureInfo &&info) {
    uint8_t* data = new uint8_t[info.GetRowStride() * info.GetHeight()];
    return std::make_shared<CClientTexture>(CClientTexture(std::move(info), data));
}