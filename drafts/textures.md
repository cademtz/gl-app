# Textures
Gl-app has many texture classes, all of them with different use-cases.
A texture is just an array of colors but the data may need to be stored in RAM, copied to VRAM, discarded from RAM, or loaded as-needed.

For these use-cases we have three main texture classes:

### CTextureInfo
- Stores texture size, number of channels, and bit format

### CClientTexture : *CTextureInfo*
- Texture data is stored in RAM
- May be accessed any time, for any reason
- Cannot be used by the render APIs

### CServerTexture : *CTextureInfo*
- Texture data could be stored anywhere (most likely in VRAM)
- Cannot be read or written
- Can be used by the render APIs

## Example

Scenario:
Your code receives a list of users with profile pictures over the network.

```cpp
namespace sesh {
struct User {
    string name;
    uint32_t id;
    // nullptr if no picture
    CClientTexture::Handle picture;
};

static std::list<User> userlist;
}
```

All users in `userlist` will have their pictures shown in the GUI.

The GUI keeps all textures packed into one atlas. When a texture is added, every
other texture is also re-read for the CPU to make an updated atlas.

Here is what the atlas and atlas "nodes" might look like:

```cpp
namespace gui {
// Atlas node
class CGuiTexture {
    // Texture to be copied into the atlas
    CClientTexture::Handle input;
    
    // true if the current atlas contains this texture
    bool is_packed;
    // Top-left and bottom-right of this texture inside the atlas (if packed)
    vec<2, float> m_packed_tl;
    vec<2, float> m_packed_br;
public:
    using Handle = /* ... */ ;
    /* ... */
};

class CGuiAtlas {
    // Handles will never be freed, but that is fine for now
    std::vector<CGuiTexture::Handle> m_nodes;
    
public:
    // Return true if there are unpacked nodes
    bool NeedsRepack();

    // Pack and return true if all nodes got packed
    bool Pack();
};
}
```

And here is how we might add and use these textures:

```cpp

namespace gui {
class CUserList : public CPanel {
    std::unordered_map<uint32_t, CGuiTexture::Handle> m_userpics;

    bool IsPicAdded(const sesh::User& user) const {
        return m_userpics.find(user.id) != m_userpics.cend();
    }

    void AddPic(const sesh::User& user, CGuiBuilder& builder) {
        m_userpics.insert_or_assign(
            std::make_pair(user.id, builder.AddTexture(user.picture))
        );
    }

public:
    void OnRender(CGuiBuilder& builder) override {
        for (const sesh::User& user : sesh::userlist) {
            if (!IsPicAdded(user))
                AddPic(user, builder)
            

        }
    }
};
}
```