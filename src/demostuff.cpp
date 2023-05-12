#include "demostuff.hpp"
#include "hid.hpp"
#include "mousecodes.hpp"
#include <iostream>
#include <chrono>

DemoStuff::DemoStuff()
    : m_birth_msec(std::chrono::steady_clock::now().time_since_epoch().count())
{

}

size_t DemoStuff::GetPoints(std::vector<glm::vec2>* out_points) const {
    out_points->insert(out_points->end(), m_points.begin(), m_points.end());
    return m_points.size();
}

size_t DemoStuff::GetText(std::basic_string<uint32_t>* out_text) const {
    out_text->insert(out_text->end(), m_text.begin(), m_text.end());
    return m_text.size();
}

void DemoStuff::OnPhysicalKey(hid::PhysicalKey key) {
    if (key.action == hid::KEY_ACTION_PRESS || key.action == hid::KEY_ACTION_REPEAT) {
        switch (key.code) {
        case hid::KEY_BACKSPACE:
            if (!m_text.empty())
                m_text.pop_back();
            break;
        case hid::KEY_ENTER:
            m_text += '\n';
            break;
        default:
            break;
        }
    }
}

void DemoStuff::OnCharKey(uint32_t codepoint) {
    m_text += codepoint;
}

void DemoStuff::OnMousePos(hid::MousePos pos) {
    m_cursor_pos = pos;
    if (m_mousedown_state[hid::MOUSE_LEFT]) {
        m_points.push_back(glm::vec2(m_cursor_pos.x, m_cursor_pos.y));
    }
    if (m_mousedown_state[hid::MOUSE_RIGHT]) {
        if (m_points.size())
            m_points.pop_back();
    }
}

void DemoStuff::OnMouseButton(hid::MouseButton btn) {
    if (btn.code < m_mousedown_state.size()) {
        m_mousedown_state[btn.code] = btn.action == hid::MOUSE_ACTION_PRESS;
    }

    if (btn.action == hid::MOUSE_ACTION_PRESS) {
        switch (btn.code) {
        case hid::MOUSE_LEFT:
            m_points.push_back(glm::vec2(m_cursor_pos.x, m_cursor_pos.y));
            break;
        case hid::MOUSE_RIGHT:
            if (m_points.size())
                m_points.pop_back();
            break;
        }
    }
}

