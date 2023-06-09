#include "demostuff.hpp"
#include "glm/gtx/matrix_transform_2d.hpp"
#include "input/hid.hpp"
#include "input/inputhandler.hpp"
#include "input/mousecodes.hpp"
#include "platform.hpp"
#include "render/sticks/drawlist.hpp"
#include <iostream>
#include <chrono>

#include <controls/panel.hpp>
#include <controls/button.hpp>
#include <memory>

#include <glm/ext/matrix_transform.hpp>

using namespace controls;

DemoStuff::DemoStuff() : m_root_control_pos(0, 100) {
    auto panel = std::make_unique<Panel>(Panel::LayoutDir::TOP_TO_BOTTOM, glm::vec2(64, 256));

    const glm::vec2 btn_size = glm::vec2(50.f);

    // add_btn
    Control::Ptr button = std::make_unique<Button>(
        [this](auto& arg) {
            AddFunShape();
        },
        [](auto& arg){},
        btn_size
    );
    panel->AddChild(std::move(button));

    // remove_btn
    button = std::make_unique<Button>(
        [this](auto& arg) { RemoveFunShape(); },
        [](auto& arg){},
        btn_size
    );
    panel->AddChild(std::move(button));

    m_root_control = std::move(panel);
}

void DemoStuff::AddFunShape() {
    m_shapes.emplace_back();
    std::cout << "AddFunShape" << std::endl;
}

void DemoStuff::RemoveFunShape() {
    if (!m_shapes.empty())
        m_shapes.pop_back();
    m_active_vertex = nullptr;
    std::cout << "RemoveFunShape" << std::endl;
}

void DemoStuff::OnMousePos(hid::MousePos pos) {
    m_cursor_pos = pos;

    if (m_active_vertex)
        m_active_vertex->x = pos.x, m_active_vertex->y = pos.y;
}

void DemoStuff::OnMouseButton(hid::MouseButton btn) {
    if (btn.code < m_mousedown_state.size()) {
        m_mousedown_state[btn.code] = btn.action == hid::MOUSE_ACTION_PRESS;
    }

    if (btn.code == hid::MOUSE_LEFT) {
        if (m_mousedown_state[btn.code]) {
            if (!m_active_vertex)
                m_active_vertex = FindHoveredVertex();
        } else {
            m_active_vertex = nullptr;
        }
    }

    if (btn.code == hid::MOUSE_RIGHT && m_mousedown_state[btn.code]) {
        auto hovered_vert = FindHoveredVertex();
        for (auto& shape : m_shapes) {
            for (auto& vert : shape.vertices) {
                if (hovered_vert == &vert)
                    shape.SetConvex(vert.convex != 1);
            }
        }
    }
}

void DemoStuff::RunEvent(const hid::Event& event) {
    // TODO: Focus/Hit-test function
    m_root_control->RunEvent(event, m_root_control_pos.x, m_root_control_pos.y);
    hid::InputHandler::RunEvent(event);
}

void DemoStuff::DrawGui(gui::Draw& draw) {
    // Draw shape handles
    const sticks::Vertex* hovered_vert = FindHoveredVertex();
    for (auto& shape : m_shapes) {
        for (auto& vert : shape.vertices) {
            glm::vec4 color = glm::vec4(1, 0.5, 0, 0.5);
            if (&vert == m_active_vertex)
                color = glm::vec4(1, 0.75, 0, 1);
            else if (&vert == hovered_vert)
                color = glm::vec4(1, 0.75, 0, 0.75);
            
            const glm::vec2 radius(5);
            draw.SetColor(color);
            draw.Ellipse(8, glm::vec2(vert.x, vert.y) - radius, radius * 2.f);
        }
    }

    m_root_control->Draw(draw, m_root_control_pos.x, m_root_control_pos.y);
}

void DemoStuff::DrawSticks(sticks::Draw& draw) {
    int screen_w, screen_h;
    Platform::GetFrameBufferSize(&screen_w, &screen_h);

    // Screen-space to normalized
    glm::mat3x3 m(1.f);
    m = glm::translate(m, glm::vec2(-1, 1));
    m = glm::scale(m, glm::vec2(2.f / screen_w,  -2.f / screen_h));
    draw.SetTransform(m);
    
    for (const FunShape& s : m_shapes)
        draw.Raw(&s.vertices[0]);
}

sticks::Vertex* DemoStuff::FindHoveredVertex(float radius) {
    for (auto& shape : m_shapes) {
        for (auto& vert : shape.vertices) {
            float distance = glm::distance(glm::vec2(m_cursor_pos.x, m_cursor_pos.y), glm::vec2(vert.x, vert.y));
            if (distance <= radius)
                return &vert;
        }
    }
    return nullptr;
}
