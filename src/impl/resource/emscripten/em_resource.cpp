#include "em_resource.hpp"
#include <emscripten/fetch.h>
#include <iostream>

#define RES_PATH_PREFIX "resources/"

void downloadSucceeded(emscripten_fetch_t *fetch) {
  printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
  //printf("%.*s", (int)fetch->numBytes, fetch->data);
  // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
  emscripten_fetch_close(fetch); // Free data associated with the fetch.
}

void downloadFailed(emscripten_fetch_t *fetch) {
  printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
  emscripten_fetch_close(fetch); // Also free data on failure.
}

std::shared_ptr<CResource> CResource::Load_Impl(const std::string& Path)
{
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.onsuccess = &downloadSucceeded;
    attr.onerror = &downloadFailed;
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    emscripten_fetch_t* fetch = emscripten_fetch(&attr, Path.c_str()); // Begins an asynchronous fetch
    /*
    std::cout << "Result code: " << ret << std::endl;

    if (fetch->status != 200) {
        std::cout << "Fetch status " << fetch->status << std::endl;
        return nullptr;
    }

    std::shared_ptr<CResource_Impl> newRes = std::make_shared<CResource_Impl>(fetch);

    std::cout << "Downloaded " << newRes->Length() << " bytes from " << Path << std::endl;

    return newRes;*/
    return nullptr;
}

CEmscriptenResource::~CEmscriptenResource() {
    emscripten_fetch_close(m_fetch);
}