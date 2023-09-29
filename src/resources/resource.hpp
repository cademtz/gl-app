#pragma once
#include <string>
#include <memory>
#include <cstdint>
#include <functional>

/**
 * @brief Load and access data provided with the program
 */
class Resource
{
public:
    using Ptr = std::shared_ptr<Resource>;
    using LoadCallback = std::function<void(Ptr res)>;
    
    Resource(Resource&&) = default;
    virtual ~Resource() {}

    /**
     * @brief Load a resource, waiting until completion
     * @return The resource, or `nullptr` on failure
     */
    static Ptr Load(const std::string& url);
    /**
     * @brief Load a resource and call 
     * @param url A local URL to the resource. Must not begin with a slash.
     * @param notify_failure If `true`, then any failure will call `callback` with a `nullptr` resource.
     * @param callback Called with the resource when it is loaded.
     * @return `nullptr` if the resource could not be loaded
     */
    static void LoadAsync(const std::string& url, bool notify_failure, LoadCallback callback);
    /** @return Resource data as signed bytes */
    virtual const int8_t* Data() const { return (const int8_t*)m_data; };
    /** @return Resource data as unsigned bytes */
    const uint8_t* UData() const { return (const uint8_t*)m_data; }
    /** @return Resource length in bytes */
    virtual size_t Length() const { return m_len; }

protected:
    Resource(const void* data, size_t data_len) : m_data(data), m_len(data_len) {}

    static Ptr FindExisting(const std::string& url);
    /**
     * @brief Load a copy of the resource in memory
     * @return `nullptr` if the resource could not be loaded
     */
    static void LoadAsyncInternal(const std::string& url, bool notify_failure, LoadCallback callback);
    /**
     * @brief Load a copy of the resource in memory
     * @return `nullptr` if the resource could not be loaded
     */
    static Ptr LoadInternal(const std::string& url);

private:
    const void* const m_data;
    const size_t m_len;
    Resource(const Resource&) = delete;
};