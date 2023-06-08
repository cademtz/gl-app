#pragma once
#include <string>

class CRender
{
public:
    CRender(const std::string& debug_name) : m_debugName(debug_name) { }
    virtual ~CRender() { }
    
    virtual void Render() = 0;
    
    const std::string m_debugName;
};