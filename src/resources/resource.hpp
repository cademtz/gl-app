#pragma once
#include <string>
#include <memory>
#include <unordered_map>

/**
 * @brief Load and access data provided with the program
 */
class CResource
{
public:
    CResource(CResource&&) = default;

    /**
     * @brief Loads a copy of the resource in memory or returns an existing pointer
     *
     * @param Name A relative URL identifying the resource to load
     * @return A reuseable resource pointer, or `nullptr` if the resource could not be loaded
     */
    static const std::shared_ptr<CResource> Load(const std::string& Name);

    /**
     * @return Array of bytes contained in the resource
     */
    virtual const char* Data() const = 0;

    /**
     * @return Length of the byte array returned by Data()
     */
    virtual size_t Length() const = 0;

protected:
    CResource() {}

    /**
     * @brief Load a copy of the resource in memory
     * 
     * @param Path A relative URL identifying the resource to load
     * @return A reusable resource pointer, or `nullptr` if the resource could not be loaded
     */
    static std::shared_ptr<CResource> Load_Impl(const std::string& Path);

    static std::unordered_map<std::string, std::shared_ptr<CResource>> loadedResMap;

private:
    CResource(const CResource&) = delete;
};