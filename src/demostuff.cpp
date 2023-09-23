#include "demostuff.hpp"
#include "glm/ext/vector_float2.hpp"
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

static gui::FontHandle btn_font = gui::FontManager::CreateFont(FontBakeConfig("Open_Sans/static/OpenSans-Regular.ttf", 16));

static glm::vec2 GetQuadraticPoint(float t, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) {
    //glm::vec2 interp_a2b = a + (b-a) * t;
    //glm::vec2 interp_b2c = b + (c-b) * t;
    //glm::vec2 interp_final = interp_a2b + (interp_b2c - interp_a2b) * t;
    glm::vec2 interp_final = a + t* (-2.f*a + 2.f*b) + t*t*(a - 2.f*b + c);
    return interp_final;
}

DemoStuff::DemoStuff() : m_root_control_pos(0, 100) {
    auto panel = std::make_unique<Panel>(Panel::LayoutDir::TOP_TO_BOTTOM, glm::vec2(64, 256));

    const glm::vec2 btn_size = glm::vec2(50.f);

    // add_btn
    auto button = std::make_unique<Button>(btn_size);
    button->SetText(U"Add").SetOnPress([this](auto) { AddFunShape(); }).SetFont(btn_font);
    panel->AddChild(std::move(button));

    // remove_btn
    button = std::make_unique<Button>(btn_size);
    button->SetText(U"Remove").SetOnPress([this](auto) { RemoveFunShape(); }).SetFont(btn_font);
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
    const glm::vec2 radius(5);

    const sticks::Vertex* hovered_vert = FindHoveredVertex();
    for (auto& shape : m_shapes) {
        std::array<glm::vec2, 3> points;
        for (size_t i = 0; i < shape.vertices.size(); ++i) {
            auto& vert = shape.vertices[i];
            points[i] = glm::vec2(vert.x, vert.y);

            glm::vec4 color = glm::vec4(1, 0.5, 0, 0.5);
            if (&vert == m_active_vertex)
                color = glm::vec4(1, 0.75, 0, 1);
            else if (&vert == hovered_vert)
                color = glm::vec4(1, 0.75, 0, 0.75);
            
            draw.SetColor(color);
            draw.Ellipse(8, points[i] - radius, radius * 2.f);
        }

        draw.SetColor(0.5, 0.5, 1);
        for (float t = 0; t < 1; t += 1.f/2) {
            if (t == 0)
                continue;
            glm::vec2 mid_point = GetQuadraticPoint(t, points[0], points[1], points[2]);
            draw.Ellipse(8, mid_point - radius, radius * 2.f);
        }
    }

    // Draw control panel
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

    draw.Segment(
        sticks::Point{glm::vec2(100, 128), sticks::SegmentCap::CIRCLE, 13, glm::vec4(1.f)},
        sticks::Point{glm::vec2(23, 59), sticks::SegmentCap::CIRCLE, 13, glm::vec4(1, 0, 0, 1)}
    );
    
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
