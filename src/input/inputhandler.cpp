#include "inputhandler.hpp"
#include "input/hid.hpp"

namespace hid {

/**
 * @brief Overloads `operator()` for every type in the @ref Event variant.
 * If this doesn't provide a necessary overload, then it should be added.
 */
struct EventToInputHandler {
    InputHandler& handler;
    
    void operator()(const hid::MouseButton& value)     { handler.OnMouseButton(value); }
    void operator()(const hid::MousePos& value)        { handler.OnMousePos(value); }
    void operator()(const hid::Scroll& value)          { handler.OnScroll(value); }
    void operator()(const hid::PhysicalKey& value)     { handler.OnPhysicalKey(value); }
    void operator()(const hid::CharacterKey& value)    { handler.OnCharKey(value); }
    void operator()(const hid::SingleTouch& value)     { handler.OnSingleTouch(value); }
    void operator()(const hid::MultiTouch& value)      { handler.OnMultiTouch(value); }
};

void InputHandler::RunEvent(const Event& event) {
    std::visit(EventToInputHandler{*this}, event);
}

}