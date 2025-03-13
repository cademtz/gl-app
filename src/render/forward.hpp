#pragma once
#include <memory>

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

class Texture;
class ClientTexture;
using TexturePtr = std::shared_ptr<Texture>;
using ClientTexturePtr = std::shared_ptr<ClientTexture>;
using ClientTextureConstPtr = std::shared_ptr<const ClientTexture>;
