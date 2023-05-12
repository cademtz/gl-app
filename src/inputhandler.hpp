#pragma once
#include "hid.hpp"

class InputHandler {
public:
    virtual void OnMouseButton(hid::MouseButton btn) {}
    virtual void OnMousePos(hid::MousePos pos) {}
    virtual void OnScroll(hid::Scroll scroll) {}
    virtual void OnPhysicalKey(hid::PhysicalKey key) {}
    virtual void OnCharKey(uint32_t codepoint) {}
    virtual void OnSingleTouch(hid::SingleTouch) {}
    virtual void OnMultiTouch(hid::MultiTouch touches) {}

    virtual ~InputHandler() {};
};