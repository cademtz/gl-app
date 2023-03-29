#pragma once
#include <resources/resource.hpp>

/**
 * Load resources from files within the current directory
*/
class CFileSystemResource : public CResource {
public:
    static std::shared_ptr<CFileSystemResource> Load(const std::string& Path);

    CFileSystemResource(CFileSystemResource&&) = default;

    ~CFileSystemResource() {
        delete[] m_data;
    }

    const char* Data() const override { return m_data; }
    size_t Length() const override { return m_len; } 

private:
    CFileSystemResource(char* Data, size_t Length) : m_data(Data), m_len(Length) {}

    char* m_data;
    size_t m_len;
};