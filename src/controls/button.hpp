#pragma once
#include "control.hpp"
#include <functional>
#include <vector>
#include <string>

namespace controls {

class Button : public Control {
public:
    using ButtonHandler = std::function<void(Button&)>;

    Button(ButtonHandler on_press, ButtonHandler on_release, Size size);
    
    void SetText(std::basic_string<uint32_t> text);
    void SetFont(gui::Draw::Font font);
    void DrawImpl(gui::Draw& draw, int32_t x, int32_t y) override;
    void OnMousePos(hid::MousePos pos) override;
    void OnMouseButton(hid::MouseButton btn) override;
    bool ShouldRedraw() const override;

    void OnPress();
    void OnRelease();

private:
    struct DrawState {
        bool is_hovered = false;
        bool is_down = false;

        bool operator==(const DrawState& other) const {
            return is_hovered == other.is_hovered && is_down == other.is_down;
        }
        bool operator!=(const DrawState& other) const { return !(*this == other); }
    };

    // The latest state. May change at any moment.
    DrawState m_draw;
    // The previous state. May change when the control is drawn.
    DrawState m_draw_prev;
    // Called when pressed/released, respectively
    ButtonHandler m_on_press, m_on_release;

    std::basic_string<uint32_t> m_text;
    
    gui::Draw::Font m_font = nullptr;
};

}
