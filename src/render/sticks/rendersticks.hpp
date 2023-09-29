#pragma once
#include <memory>
#include <render/render.hpp>

namespace sticks {

class DrawList;

class RenderSticks : public CRender {
public:    
    static std::shared_ptr<RenderSticks> GetInstance();
    virtual void UploadDrawData(const DrawList& list) = 0;
};

}