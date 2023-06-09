#pragma once
#include "control.hpp"

namespace controls {

class Panel : public Control {
public:
    enum class LayoutDir {
        LEFT_TO_RIGHT,
        TOP_TO_BOTTOM,
    };
    
    Panel(LayoutDir direction, Size layout_size);

    void AddChild(Control::Ptr control);
    void RunEvent(const hid::Event& event) override;

protected:
    void DrawImpl(gui::Draw& draw, int32_t x, int32_t y) override;

    glm::vec<2, int32_t> GetChildOffset(size_t index) const;

private:
    LayoutDir m_layout_dir;
};

}