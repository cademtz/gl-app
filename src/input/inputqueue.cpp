#include "inputqueue.hpp"
#include "input/hid.hpp"

namespace hid {

void InputQueue::OnMouseButton(hid::MouseButton btn)    { AddEvent(btn); }
void InputQueue::OnMousePos(hid::MousePos pos)          { AddEvent(pos); }
void InputQueue::OnScroll(hid::Scroll scroll)           { AddEvent(scroll); }
void InputQueue::OnPhysicalKey(hid::PhysicalKey key)    { AddEvent(key); }
void InputQueue::OnCharKey(hid::CharacterKey key)       { AddEvent(key); }
void InputQueue::OnSingleTouch(hid::SingleTouch touch)  { AddEvent(touch); }
void InputQueue::OnMultiTouch(hid::MultiTouch touches)  { AddEvent(touches); }

void InputQueue::Flush(EventCallback visit) {
    std::lock_guard g(m_mutex);
    for (const Event& event : m_events)
        visit(event);
    m_events.clear();
}

void InputQueue::AddEvent(const Event& event) {
    std::lock_guard g(m_mutex);
    m_events.push_back(event);
}

}