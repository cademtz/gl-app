#include "control.hpp"
#include "input/hid.hpp"
#include "input/inputhandler.hpp"
#include <memory>

namespace controls {

Control::Control(Size layout_size) {
    SetLayoutSize(layout_size);
}

void Control::Draw(gui::Draw& draw, int32_t x, int32_t y) {
    m_update_draw = false;
    DrawImpl(draw, x, y);
}

void Control::RunEvent(const hid::Event& event, int32_t x, int32_t y) {
    RunEvent(event.Translate(-x, -y));
}

void Control::RunEvent(const hid::Event& event) {
    hid::InputHandler::RunEvent(event);
}

void Control::AddChild(Control::Ptr control) {
    m_children.emplace_back(std::move(control));
}

bool Control::ShouldRedraw() const {
    if (m_update_draw)
        return true;
    
    for (auto& child_ptr : m_children) {
        if (child_ptr->ShouldRedraw())
            return true;
    }
    return false;
}

void Control::SetShouldRedraw() { m_update_draw = true; }

bool Control::ShouldUpdateLayout() const {
    if (m_update_layout)
        return true;
    
    for (auto& child_ptr : m_children) {
        if (child_ptr->ShouldUpdateLayout())
            return true;
    }
    return false;
}

void Control::SetLayoutSize(Size new_size) {
    m_update_layout = true;
    m_layout_size = new_size;
}

void Control::OnUpdateLayout() {
    m_update_layout = false;
}

}