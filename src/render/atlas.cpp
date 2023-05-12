#include "atlas.hpp"
#include "render/texture.hpp"
#include "bake.hpp"
#include <utility>

#include <cassert>

CTextureLoader::Ptr CTextureLoader::FromClientTexture(ClientTexture::ConstPtr texture) {
    return CClientTextureInput::Create(texture);
}

CTextureAtlas::CTextureAtlas(TextureFormat format, CTextureLoader::Ptr default_texture)
    : m_format(format), m_default_tex(default_texture) {
    
}

void CTextureAtlas::AddTexture(CTextureLoader::Ptr input) {
    if (!m_is_packed)
        m_inputs.emplace_back(input);
}

bool CTextureAtlas::Pack() {
    if (m_is_packed)
        return true;
    m_is_packed = true;

    RectPacker rp;
    std::unordered_map<CTextureLoader*, uint32_t> m_rectmap;

    // Add default texture to rect packer
    const TextureInfo& default_info = m_default_tex->GetTextureInfo();
    uint32_t default_rect_id = rp.AddRect(default_info.GetWidth(), default_info.GetHeight());
    
    // Add bounds of all input textures to rect packer
    for (const auto& loader : m_inputs) {
        TextureInfo info = loader->GetTextureInfo();
        uint32_t rect_id = rp.AddRect(info.GetWidth(), info.GetHeight());
        m_rectmap.emplace(std::make_pair(loader.get(), rect_id));
    }

    if (!rp.Pack())
        return false;
    
    // Allocate new atlas and new nodes map
    uint32_t atlas_w, atlas_h;
    rp.GetPackedSize(&atlas_w, &atlas_h);
    ClientTexture::Ptr new_atlas = ClientTexture::Create(TextureInfo(m_format, atlas_w, atlas_h));
    std::unordered_map<const CTextureLoader*, Node> new_nodes;

    // Get default rect
    RectPacker::Rect default_rect;
    rp.GetRect(default_rect_id, &default_rect);

    // Write default texture into the atlas
    new_atlas->Write(m_default_tex->GetTexture(), default_rect.x, default_rect.y);

    // Write every other texture into the atlas
    for (auto& loader : m_inputs) {
        RectPacker::Rect rect;
        rp.GetRect(m_rectmap[loader.get()], &rect);

        Node node;
        node.x = rect.x, node.y = rect.y;
        node.w = rect.w, node.h = rect.h;
        new_nodes.emplace(std::make_pair(loader.get(), node));

        ClientTexture::ConstPtr temp_texture = loader->GetTexture();
        assert(temp_texture->GetWidth() == loader->GetTextureInfo().GetWidth()
            && temp_texture->GetHeight() == loader->GetTextureInfo().GetHeight());
        if (!new_atlas->Write(temp_texture, rect.x, rect.y))
            return false;
    }

    // Assign the new texture mappings and and trash the inputs
    m_inputs.clear();
    m_nodes = new_nodes;
    m_packed_tex = Texture::Create(new_atlas);
    m_default_node = {
        (uint32_t)default_rect.x, (uint32_t)default_rect.y,
        (uint32_t)default_rect.w, (uint32_t)default_rect.h
    };

    m_is_packed = true;
    return m_is_packed;
}

Texture::Ptr CTextureAtlas::GetPackedTexture() const {
    return m_packed_tex;
}

bool CTextureAtlas::GetTextureRect(CTextureLoader::Ptr input, Node *out_rect) const {
    if (!IsPacked()) {
        *out_rect = m_default_node;
        return false;
    }

    auto it = m_nodes.find(input.get());
    if (it == m_nodes.cend()) {
        *out_rect = m_default_node;
        return false;
    }
    
    *out_rect = it->second;
    return true;
}