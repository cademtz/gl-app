#include "libcxx_resource.hpp"
#include <fstream>

#define RES_PATH_PREFIX _resPathPrefix.c_str()

static std::string ResPathHack()
{
    std::string path = __FILE__;
    size_t end = path.rfind("\\src\\");
    if (end == std::string::npos)
        end = path.rfind("//src//");
    path = path.substr(0, end + 1) + "resources/";
    return path;
}
static std::string _resPathPrefix = ResPathHack();

std::shared_ptr<CResource> CResource::Load_Impl(const std::string& Path)
{
    std::fstream file(Path, std::ios::in | std::ios::binary);
    if (!file.is_open())
        return false;

    file.seekg(0, std::ios::end);

    size_t m_len = file.tellg();
    if (file.fail())
        return false;
    
    char* data = new char[m_len];

    file.seekg(0, std::ios::beg);
    file.read(data, m_len);
    const char* m_data = reinterpret_cast<const char*>(data);

    return std::make_shared<CResource>(m_data, m_len);
}