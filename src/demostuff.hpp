#pragma once
#include <glm/vec2.hpp>
#include <memory>
#include <string>
#include <cstdint>
#include <vector>
#include <array>
#include "hid.hpp"
#include "inputhandler.hpp"
#include "mousecodes.hpp"

class DemoStuff : public InputHandler {
public:
    DemoStuff();

    InputHandler* GetInputHandler() { return this; }
    size_t GetPoints(std::vector<glm::vec2>* out_points) const;
    size_t GetText(std::basic_string<uint32_t>* out_text) const;

    void OnPhysicalKey(hid::PhysicalKey key) override;
    void OnCharKey(uint32_t codepoint) override;
    void OnMousePos(hid::MousePos pos) override;
    void OnMouseButton(hid::MouseButton btn) override;

private:
    std::array<bool, 8> m_mousedown_state = { false };
    const uint64_t m_birth_msec;

    hid::MousePos m_cursor_pos;
    std::vector<glm::vec2> m_points;
    std::basic_string<uint32_t> m_text;
};