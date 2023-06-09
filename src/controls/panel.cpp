#include "panel.hpp"
#include "glm/ext/vector_float4.hpp"
#include "input/inputhandler.hpp"

namespace controls {

Panel::Panel(LayoutDir direction, Size layout_size)
    : m_layout_dir(direction), Control(layout_size) {}

void Panel::AddChild(Control::Ptr control) {
    Control::AddChild(std::move(control));
}

void Panel::RunEvent(const hid::Event& event) {
    Control::RunEvent(event);
    for (size_t i = 0; i < m_children.size(); ++i) {
        auto& child = m_children[i];
        auto child_off = GetChildOffset(i);
        child->RunEvent(event, child_off.x, child_off.y);
    }
}

void Panel::DrawImpl(gui::Draw& draw, int32_t x, int32_t y) {
    Size size = GetLayoutSize();

    draw.PushClip(glm::vec4(x, y, size));
    draw.SetColor(0.5, 0.5, 1, 0.5);
    draw.Rect(x, y, size.x, size.y);

    for (size_t i = 0; i < m_children.size(); ++i) {
        auto& child = m_children[i];
        auto child_off = GetChildOffset(i);
        child->Draw(draw, x + child_off.x, y + child_off.y);
    }

    draw.PopClip();
}

glm::vec<2, int32_t> Panel::GetChildOffset(size_t index) const {
    Size size = GetLayoutSize();

    const glm::vec<2, int32_t> padding = {10, 10};

    glm::vec<2, int32_t> next_pos = {0, 0};
    uint32_t block_width = 0, block_height = 0;
    for (size_t i = 0; i < m_children.size() && i < index; ++i) {
        const auto& child = m_children[i];
        Size child_size = child->GetLayoutSize();
        if (child_size.x == 0 || child_size.y == 0)
            continue;
        
        switch (m_layout_dir) {
            case LayoutDir::LEFT_TO_RIGHT:
                next_pos.x += padding.x + child_size.x;
                break;
            case LayoutDir::TOP_TO_BOTTOM:
                next_pos.y += padding.y + child_size.y;
                break;
        }
        // TODO: "line"-wrapping when 2 or more children extend over our layout size
    }

    return next_pos;
}

}