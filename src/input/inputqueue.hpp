#pragma once
#include "hid.hpp"
#include "inputhandler.hpp"
#include <vector>
#include <mutex>

namespace hid {

class InputQueue : public InputHandler {
public:
    using EventCallback = void(const Event& event);

    void OnMouseButton(hid::MouseButton btn);
    void OnMousePos(hid::MousePos pos);
    void OnScroll(hid::Scroll scroll);
    void OnPhysicalKey(hid::PhysicalKey key);
    void OnCharKey(hid::CharacterKey key);
    void OnSingleTouch(hid::SingleTouch touch);
    void OnMultiTouch(hid::MultiTouch touches);

    /**
     * @brief Clear all events from the queue while visiting each one along the way
     * @param visit Called for every event in the queue, in order
     */
    void Flush(EventCallback visit);
    void AddEvent(const Event& event);

private:
    std::mutex m_mutex;
    std::vector<Event> m_events;
};

}