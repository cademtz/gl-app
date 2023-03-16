#include "resource.hpp"
#include <string.h>

std::unordered_map<std::string, std::shared_ptr<CResource>> CResource::loadedResMap;

const std::shared_ptr<CResource> CResource::Load(const std::string& Name)
{
    auto it = loadedResMap.find(Name);
    if (it != loadedResMap.end())
        return it->second;
    
    std::shared_ptr<CResource> res = CResource::Load_Impl(Name.c_str());
    if (!res)
        return nullptr;

    it = loadedResMap.emplace(Name, std::move(res)).first;
    return it->second;
}