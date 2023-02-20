#include "resource.h"
#include <string.h>

std::map<std::string, std::shared_ptr<CResource>> CResource::loadedResMap;

#ifdef __EMSCRIPTEN__
#include <emscripten/fetch.h>
#include <iostream>

#define RES_PATH_PREFIX "resources/"

bool CResource::LoadFile(const char* Path)
{
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
    m_fetch = emscripten_fetch(&attr, Path); // Blocks here until the operation is complete.
    if (m_fetch->status != 200)
        return false;
    
    m_data = m_fetch->data;
    m_len = m_fetch->numBytes;

    std::cout << "Downloaded " << m_len << " bytes from " << Path << std::endl;

    return true;
}
#else
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

bool CResource::LoadFile(const char* Path)
{
    std::fstream file(Path, std::ios::in | std::ios::binary);
    if (!file.is_open())
        return false;

    file.seekg(0, std::ios::end);

    m_len = file.tellg();
    if (file.fail())
        return false;
    
    char* data = new char[m_len];

    file.seekg(0, std::ios::beg);
    file.read(data, m_len);
    m_data = (const char*)data;

    return true;
}

#endif

CResource::~CResource()
{
#ifdef __EMSCRIPTEN__
    emscripten_fetch_close(m_fetch);
#else
    if (m_data)
        delete[] m_data;
#endif
}

const std::shared_ptr<CResource> CResource::Load(const std::string& Name)
{
    auto it = loadedResMap.find(Name);
    if (it != loadedResMap.end())
        return it->second;
    
    std::shared_ptr<CResource> res = std::make_shared<CResource>(CResource());
    if (!res->LoadFile((RES_PATH_PREFIX + Name).c_str()))
        return std::shared_ptr<CResource>();

    it = loadedResMap.emplace(Name, std::move(res)).first;
    return it->second;
}