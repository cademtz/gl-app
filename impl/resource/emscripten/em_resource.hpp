#include <resources/resource.hpp>
#include <emscripten/fetch.h>

class CResource_Impl : public CResource {
public:
    CResource_Impl(emscripten_fetch_t* fetch) : m_fetch(fetch) {}
    ~CResource_Impl();

    virtual const char* Data() const {
        return m_fetch->data;
    }

    virtual size_t Length() const {
        return m_fetch->numBytes;
    }

    emscripten_fetch_t* m_fetch;
};