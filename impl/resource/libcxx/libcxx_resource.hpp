#pragma once
#include <resources/resource.hpp>

class CResource_Impl : public CResource {
public:
    ~CResource_Impl() {
        delete[] m_data;
    }

private:
    CResource_Impl(char* Data, size_t Length) : m_data(Data), m_len(Length) {}

    char* m_data;
    size_t m_len;
};