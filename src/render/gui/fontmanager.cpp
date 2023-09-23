#include "fontmanager.hpp"
#include "fontatlas.hpp"
#include <unordered_map>
#include <stack>
#include <platform.hpp>
#include <resources/resource.hpp>

namespace gui {

static bool g_cleanup = false;

// The following functions are declared to work around static initialization ordering
static auto& GetAtlasMap() {
    static std::unordered_map<_FontHandle*, FontAtlas> m;
    return m;
}
static auto& GetQueue() {
    static std::stack<FontHandle> q;
    return q;
}

/**
 * @brief Handle that maps to a baked font atlas.
 * - The handle should be stored only as a shared_ptr reference.
 * - The handle must not be copied (its memory addresss makes it unique).
 * - The shared_ptr must not be owned by its gui::Draw instance (or else it only dies when gui::Draw dies).
 */
struct _FontHandle
{
    _FontHandle(FontBakeConfig&& config) : config(std::move(config)) {}
    _FontHandle(const _FontHandle&) = delete;
    ~_FontHandle() {
        if (!g_cleanup)
            GetAtlasMap().erase(this);
    }

    FontBakeConfig config;
};

FontHandle FontManager::CreateFont(FontBakeConfig&& config) {
    FontHandle handle = std::make_shared<_FontHandle>(std::move(config));
    GetQueue().push(handle);
    return handle;
}

const FontAtlas* FontManager::GetAtlas(FontHandle handle) {
    auto it = GetAtlasMap().find(handle.get());
    if (it != GetAtlasMap().end())
        return &it->second;
    return nullptr;
}

void FontManager::RunQueue() {
    while (!GetQueue().empty()) {
        FontHandle handle = GetQueue().top();
        GetQueue().pop();

         CResource::Ptr res = CResource::LoadSynchronous(handle->config.url);
        if (!res) {
            PLATFORM_WARNING("res == nullptr");
            continue;
        }

        std::optional<TrueType> tt = TrueType::FromTrueType(res);
        if (!tt) {
            PLATFORM_WARNING("failed to parse truetype");
            continue;
        }

        GetAtlasMap().emplace(std::make_pair(handle.get(), FontAtlas(*tt, handle->config)));
    }
}

void FontManager::Cleanup() {
    g_cleanup = true;
}

}