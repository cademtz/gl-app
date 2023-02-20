#pragma once
#include <map>
#include <string>
#include <memory>
#include <stdint.h>

struct CResource
{
public:
    ~CResource();

    static const std::shared_ptr<CResource> Load(const std::string& Name);

    const char* Data() const { return m_data; }
    size_t Len() const { return m_len; }

private:
    CResource() { }
    bool LoadFile(const char* Path);
    
    static std::map<std::string, std::shared_ptr<CResource>> loadedResMap;

    const char* m_data = nullptr;
    size_t m_len = 0;

#ifdef __EMSCRIPTEN__
    struct emscripten_fetch_t* m_fetch;
#endif
};