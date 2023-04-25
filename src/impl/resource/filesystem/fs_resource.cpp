#include "fs_resource.hpp"
#include <fstream>

#define RES_PATH_PREFIX _resPathPrefix.c_str()

static std::string ResPathHack()
{
    std::string path = __FILE__;
    size_t end = path.rfind("\\src\\");
    if (end == std::string::npos)
        end = path.rfind("/src/");
    path = path.substr(0, end + 1) + "resources/";
    return path;
}
static std::string _resPathPrefix = ResPathHack();

std::shared_ptr<CFileSystemResource> CFileSystemResource::Load(const std::string& Path)
{
    std::string fs_path = _resPathPrefix + Path;
    std::fstream file(fs_path, std::ios::in | std::ios::binary);
    if (!file.is_open())
        return nullptr;

    file.seekg(0, std::ios::end);

    size_t len = file.tellg();
    if (file.fail())
        return nullptr;
    
    char* data = new char[len];

    file.seekg(0, std::ios::beg);
    file.read(data, len);

    return std::make_shared<CFileSystemResource>(data, len);
}


std::shared_ptr<CResource> CResource::LoadSynchronous_Impl(const std::string& Path) {
    return CFileSystemResource::Load(Path);
}