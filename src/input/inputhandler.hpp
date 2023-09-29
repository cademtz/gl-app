#pragma once
#include "hid.hpp"
#include "event.hpp"

namespace hid {

struct EventToInputHandler;

/**
 * @brief Process input with overrideable functions.
 */
class InputHandler {
public:
    /** Call the appropriate input handler for the event type */
    virtual void RunEvent(const Event& event);

    virtual ~InputHandler() {}

protected:
    friend EventToInputHandler;

    virtual void OnMouseButton(hid::MouseButton btn) {}
    virtual void OnMousePos(hid::MousePos pos) {}
    virtual void OnScroll(hid::Scroll scroll) {}
    virtual void OnPhysicalKey(hid::PhysicalKey key) {}
    virtual void OnCharKey(hid::CharacterKey key) {}
    virtual void OnSingleTouch(hid::SingleTouch touch) {}
    virtual void OnMultiTouch(hid::MultiTouch touches) {}
};

}