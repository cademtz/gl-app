#pragma once
#include <resources/resource.hpp>
#include <emscripten/fetch.h>

// TODO: Cache certain resources to prevent unecessary bandwidth?
//  Will require fetch, which is near-impossible to use due to poor async/thread compatibility.
/**
 * Download resources from the website currently serving this app.
*/
class CEmscriptenResource : public CResource {
public:
    CEmscriptenResource(emscripten_fetch_t* fetch) : m_fetch(fetch) {}
    ~CEmscriptenResource();

    virtual const char* Data() const {
        return m_fetch->data;
    }

    virtual size_t Length() const {
        return m_fetch->numBytes;
    }

    emscripten_fetch_t* m_fetch;
};