#include "texture.hpp"
#include <algorithm>
#include <cassert>
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
    switch (GetFormat()) {
    case TextureFormat::A_8_8: return 1;
    case TextureFormat::RGB_8_24: return 3;
    case TextureFormat::RGBA_8_32: return 4;
    default:
        assert(0 && "Missing texture format case!");
    }
    return ~(uint32_t)0;
}

bool ClientTexture::Write(ClientTexture::ConstPtr new_data, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    // Mismatched formats, or texture coords are completely out of bounds
    if (GetFormat() != new_data->GetFormat() || x >= GetWidth() || y >= GetHeight())
        return false;
    
    if (w == ~(uint32_t)0)
        w = new_data->GetWidth();
    if (h == ~(uint32_t)0)
        h = new_data->GetHeight();
    
    assert(w <= new_data->GetWidth() && h <= new_data->GetHeight());

    uint32_t write_w = w;
    uint32_t write_h = h;
    if (x + write_w > GetWidth())
        write_w = GetWidth() - x;
    if (y + write_h > GetHeight())
        write_h = GetHeight() - y;

    for (uint32_t offset_y = write_h - 1; offset_y < ~(~(uint32_t)0 >> 1); --offset_y) {
        const uint8_t* src_row = new_data->GetPixel(0, offset_y);
        uint8_t* dst_row = GetPixel(x, y + offset_y);
        std::memcpy(dst_row, src_row, GetPixelStride() * write_w);
    }
    return true;
}

ClientTexture::Ptr ClientTexture::Convert(TextureFormat new_format, ExpandOp operation) {
    ClientTexture::Ptr new_tex = ClientTexture::Create(TextureInfo(new_format, GetWidth(), GetHeight()));
    std::array<uint8_t, 4> next_pixel = { 0 };
    for (uint32_t y = 0; y < GetHeight(); ++y) {
        for (uint32_t x = 0; x < GetWidth(); ++x) {
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

ClientTexture::Ptr ClientTexture::Create(const TextureInfo &info) {
    uint8_t* data = new uint8_t[info.GetRowStride() * info.GetHeight()];
    return std::make_shared<ClientTexture>(ClientTexture(info, data, &CppFreeArray));
}

ClientTexture::Ptr ClientTexture::FromImage(ImageTypeHint type_hint, const void* img_data, size_t img_size) {
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
