#pragma once
#include <vector>
#include <unordered_map>
#include "render/texture.hpp"
#include "bake.hpp"

/**
 * @brief Common interface to load a texture
 */
class CTextureLoader {
public:
    using Ptr = std::shared_ptr<CTextureLoader>;

    /**
     * @brief Get the texture info ahead of time for packing and checking format
     */
    virtual const TextureInfo& GetTextureInfo() = 0;

    /**
     * @brief Load and return the texture
     */
    virtual ClientTextureConstPtr GetTexture() = 0;

    /**
     * @brief Use an existing texture as a texture loader
     */
    static CTextureLoader::Ptr FromClientTexture(ClientTextureConstPtr texture);
};

/**
 * @brief Use an existing texture as a texture loader
 */
class CClientTextureInput : public CTextureLoader {
public:
    CClientTextureInput(ClientTextureConstPtr texture) : m_texture(texture) {}
    const TextureInfo& GetTextureInfo() override { return m_texture->GetInfo(); }
    ClientTextureConstPtr GetTexture() override { return m_texture; }

    static CTextureLoader::Ptr Create(ClientTextureConstPtr texture) {
        return std::make_shared<CClientTextureInput>(texture);
    }

private:
    const ClientTextureConstPtr m_texture;
};

/**
 * @brief Interface to pack textures
 */
class CTexturePacker {
public:
    class Input {
    public:
        using Ptr = std::shared_ptr<Input>;

        virtual ~Input() {}
        virtual void AddRects(RectPacker& rect_packer) = 0;
        virtual void WriteTextures(ClientTexture& atlas) = 0;
    };

    
private:
};

/**
 * @brief Contains many textures copied into one server-side texture.
 * It is packed once, and done packing forever after. No new textures can be added.
 * If you need to add new textures, then create a new atlas with all the new and old inputs, and swap them out.
 */
class CTextureAtlas {
public:
    using Ptr = std::shared_ptr<CTextureAtlas>;
    using ConstPtr = std::shared_ptr<const CTextureAtlas>;

    /**
     * @param format Format for the atlas texture and all input textures
     * @param default_texture A placeholder texture for unmapped nodes
     */
    CTextureAtlas(TextureFormat format, CTextureLoader::Ptr default_texture);

    TextureFormat GetFormat() const { return m_format; }
    bool IsPacked() const { return m_is_packed; }
    /**
     * @brief Add a texture to the atlas.
     * If the atlas is already packed, it will have no effect.
     */
    void AddTexture(CTextureLoader::Ptr input);

    /**
     * @brief Location of a single texture in the atlas
     */
    struct Node {
        uint32_t x, y, w, h;
    };

    /**
     * @brief Get a texture's position in the atlas.
     * If the texture is not packed, it will provide a default position.
     * @return `true` if `input` is packed inside the atlas. `false` if a default position will be given.
     */
    bool GetTextureRect(CTextureLoader::Ptr input, Node* out_rect) const;

    /**
     * @brief Get the current atlas texture
     */
    TexturePtr GetPackedTexture() const;
    
    /**
     * @brief Pack the input textures inside the atlas.
     * If the atlas is already packed, it will have no effect.
     * @return `true` if the atlas is packed 
     */
    bool Pack();

protected:
    CTextureAtlas(TextureFormat format) : m_format(format) {}

private:
    const TextureFormat m_format;
    const CTextureLoader::Ptr m_default_tex;
    Node m_default_node;

    /**
     * @brief Input textures used for packing. It is cleared after packing.
     */
    std::vector<CTextureLoader::Ptr> m_inputs;

    /**
     * @brief Map of CTextureLoader* -> Node
     */
    std::unordered_map<const CTextureLoader*, Node> m_nodes;
    bool m_is_packed = false;
    TexturePtr m_packed_tex;
};