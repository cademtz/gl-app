#pragma once
#include <string>
#include <memory>
#include <cstdint>
#include <unordered_map>

/**
 * @brief Load and access data provided with the program
 */
class CResource
{
public:
    using Ptr = std::shared_ptr<CResource>;
    
    CResource(CResource&&) = default;
    virtual ~CResource() {}

    /** Loads a resource asynchronously */
    static void Load(const std::string& url);
    /**
     * @brief Loads a resource, waiting until completion
     * @return `nullptr` if the resource could not be loaded
     */
    static const std::shared_ptr<CResource> LoadSynchronous(const std::string& url);
    /** @return Array of signed bytes contained in the resource */
    virtual const int8_t* Data() const = 0;
    /** @return Array of unsigned bytes contained in the resource */
    const uint8_t* UData() const {
        return reinterpret_cast<const unsigned char*>(Data());
    }
    /** @return Length of the byte array returned by Data() */
    virtual size_t Length() const = 0;

protected:
    CResource() {}

    static std::shared_ptr<CResource> FindExisting(const std::string& url);
    /**
     * @brief Load a copy of the resource in memory
     * @return `nullptr` if the resource could not be loaded
     */
    static std::shared_ptr<CResource> Load_Impl(const std::string& Path);
    /**
     * @brief Load a copy of the resource in memory
     * @return `nullptr` if the resource could not be loaded
     */
    static std::shared_ptr<CResource> LoadSynchronous_Impl(const std::string& Path);

    static std::unordered_map<std::string, std::shared_ptr<CResource>> loadedResMap;

private:
    CResource(const CResource&) = delete;
};