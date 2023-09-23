#include "button.hpp"
#include "glm/ext/vector_float4.hpp"
#include <glm/vec3.hpp>

namespace controls {

Button::Button(Size size) : Control(size) { }

void Button::OnMousePos(hid::MousePos pos) {
    Size size = GetLayoutSize();
    m_draw.is_hovered = pos.x >= 0 && pos.y >= 0 && pos.x < size.x && pos.y < size.y;
}

void Button::OnMouseButton(hid::MouseButton btn) {
    if (m_draw.is_down && btn.action == hid::MOUSE_ACTION_RELEASE) {
        OnRelease();
    } else if (m_draw.is_hovered) {
        if (!m_draw.is_down && btn.action == hid::MOUSE_ACTION_PRESS)
            OnPress();
    }
}

void Button::OnPress() {
    m_draw.is_down = true;
    if (m_on_press)
        m_on_press(*this);
}

void Button::OnRelease() {
    m_draw.is_down = false;
    if (m_on_release)
        m_on_release(*this);
}

Button& Button::SetOnPress(ButtonHandler callback) {
    m_on_press = callback;
    return *this;
}

Button& Button::SetOnRelease(ButtonHandler callback) {
    m_on_release = callback;
    return *this;
}

Button& Button::SetText(std::u32string text) {
    SetShouldRedraw();
    m_text = text;
    return *this;
}

Button& Button::SetFont(gui::FontHandle font) {
    SetShouldRedraw();
    m_font = font;
    return *this;
}

void Button::DrawImpl(gui::Draw& draw, int32_t x, int32_t y) {
    m_draw_prev = m_draw;

    Size size = GetLayoutSize();
    glm::vec3 rgb = glm::vec3(0.5f);

    if (m_draw.is_hovered)
        rgb = glm::vec3(0.7f);
    
    draw.SetColor(glm::vec4(rgb, 1.f));
    draw.PushClip(glm::vec4(x, y, size));
    {
        draw.Rect(x, y, size.x, size.y);
        if (!m_text.empty()) {
            draw.SetColor(glm::vec4(1.f));
            draw.TextUnicode(m_font, glm::vec2(x, y), m_text);
        }
    }
    draw.PopClip();
}

bool Button::ShouldRedraw() const {
    if (m_draw != m_draw_prev)
        return true;
    if (Control::ShouldRedraw())
        return true;
    return false;
}

}