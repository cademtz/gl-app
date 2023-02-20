#pragma once
#include "../resource.h"
#include <render/texturemanager.h>

class CFont : public CTextureStream
{
public:
    static std::shared_ptr<CFont> LoadFromResource(const std::string& Name, int FontHeight);

private:
    CFont();

    std::vector<unsigned char> m_bitmap;
};