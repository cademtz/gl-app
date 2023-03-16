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

    // FIXME: Must download *after* all C code has stopped executing.
    // Impossible to load resources as-needed.
    // If we suddenly need to rebuild a font atlas and have to reload all fonts, that ENTIRE operation must be scheduled.
    //  Everything stupid thing that relies on a resource must continue to leave all the other code to get helplessly raped by unresolved situations
    // BUT, perhaps if we ran everything through a fiber, we could interrupt execution, wait for download, and resume!!!
    // https://emscripten.org/docs/api_reference/fiber.h.html

    // Bruh, you could just do this instead
    // https://emscripten.org/docs/porting/asyncify.html#making-async-web-apis-behave-as-if-they-were-synchronous
    return nullptr;
}

CResource_Impl::~CResource_Impl() {
    emscripten_fetch_close(m_fetch);
}