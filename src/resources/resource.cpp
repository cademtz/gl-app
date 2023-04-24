#include "resource.hpp"
#include <cassert>

std::unordered_map<std::string, std::shared_ptr<CResource>> CResource::loadedResMap;

void CResource::Load(const std::string &url)
{
    assert(0 && "Async resource loading is not implemented");
}

const std::shared_ptr<CResource> CResource::LoadSynchronous(const std::string& url)
{
    std::shared_ptr<CResource> res = FindExisting(url);
    if (res)
        return res;
    
    res = CResource::LoadSynchronous_Impl(url.c_str());
    if (!res)
        return nullptr;

    auto pair = loadedResMap.emplace(url, std::move(res)).first;
    return pair->second;
}

std::shared_ptr<CResource> CResource::FindExisting(const std::string& url)
{
    auto it = loadedResMap.find(url);
    if (it != loadedResMap.end())
        return it->second;
    return nullptr;
}