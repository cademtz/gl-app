#include "resource.hpp"
#include <cassert>
#include <unordered_map>

static std::unordered_map<std::string, Resource::Ptr> cache;

Resource::Ptr Resource::Load(const std::string& url) {
    std::shared_ptr<Resource> res = FindExisting(url);
    if (res)
        return res;
    
    res = Resource::LoadInternal(url.c_str());
    if (!res)
        return nullptr;

    auto pair = cache.emplace(url, std::move(res)).first;
    return pair->second;
}

void Resource::LoadAsync(const std::string& url, bool notify_failure, LoadCallback callback) {
    LoadAsyncInternal(url, notify_failure, callback);
}

Resource::Ptr Resource::FindExisting(const std::string& url) {
    auto it = cache.find(url);
    if (it != cache.end())
        return it->second;
    return nullptr;
}