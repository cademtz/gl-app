#include <resources/resource.hpp>
#include <emscripten/fetch.h>
#include <iostream>
#include <cstdlib>

// TODO: Cache certain resources to prevent unecessary bandwidth?
//  Will require fetch, which is near-impossible to use due to poor async/thread compatibility.
/**
 * Download resources from the website currently serving this app.
*/
class EmFetch : public Resource
{
public:
    EmFetch(emscripten_fetch_t* fetch)
        : Resource(fetch->data, fetch->numBytes), m_fetch(fetch) {}
    ~EmFetch() { emscripten_fetch_close(m_fetch); }

private:
    emscripten_fetch_t* m_fetch;
};

class EmWget : public Resource
{
public:
    EmWget(void* data, int len) : Resource(data, len) {}
    ~EmWget() { free((void*)Data()); }
};

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

void Resource::LoadAsyncInternal(const std::string& url, bool notify_failure, LoadCallback callback) {
    std::string real_path = "resources/" + url;

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.onsuccess = &downloadSucceeded;
    attr.onerror = &downloadFailed;
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    emscripten_fetch_t* fetch = emscripten_fetch(&attr, real_path.c_str()); // Begins an asynchronous fetch
}

Resource::Ptr Resource::LoadInternal(const std::string& url) {
    std::string real_url = "resources/" + url;
    void* new_buffer;
    int length;
    int error;
    emscripten_wget_data(real_url.c_str(), &new_buffer, &length, &error);

    if (error != 0) {
        std::cout << "error code " << error << " downloading " << real_url << std::endl;
        return nullptr;
    }
    
    return std::make_shared<EmWget>(new_buffer, length);
}