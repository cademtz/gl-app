#pragma once
#include <string>

class CRender
{
public:
    CRender(const std::string& DebugName) : m_debugName(DebugName) { }
    virtual ~CRender() { }

    virtual bool Init() = 0;
    virtual void Render() = 0;
    
    const std::string m_debugName;
};