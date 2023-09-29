#pragma once
#include <glm/vec2.hpp>
#include <memory>
#include <cstdint>
#include <vector>
#include <array>
#include "input/hid.hpp"
#include "input/inputhandler.hpp"
#include <render/sticks/drawlist.hpp>
#include <render/sticks/draw.hpp>
#include <render/gui/draw.hpp>

struct FunShape;
class Texture;

class DemoStuff : public hid::InputHandler {
public:
    DemoStuff();

    InputHandler* GetInputHandler() { return this; }

    void OnMousePos(hid::MousePos pos) override;
    void OnMouseButton(hid::MouseButton btn) override;

    void RunEvent(const hid::Event& event) override;

    void DrawGui(gui::Draw& draw);
    void DrawSticks(sticks::Draw& draw);

private:
    void AddFunShape();
    void RemoveFunShape();
    sticks::Vertex* FindHoveredVertex(float radius = 5.f);

    std::array<bool, 256> m_mousedown_state = { false };

    std::vector<FunShape> m_shapes;
    sticks::Vertex* m_active_vertex = nullptr;
    std::shared_ptr<Texture> m_gui_texture;
    gui::Draw m_draw;

    hid::MousePos m_cursor_pos;
};

struct FunShape {
    std::array<sticks::Vertex, 3> vertices = {
        sticks::Vertex {
            0, 0,
            0, 0,
            1, 1, 1, 1,
            1
        },
        sticks::Vertex {
            100, 0,
            0.5, 0,
            1, 0, 0, 1,
            1
        },
        sticks::Vertex {
            100, 100,
            1, 1,
            0, 1, 0, 1,
            1
        },
    };

    void SetConvex(bool convex) {
        for (auto& v : vertices)
            v.convex = convex ? 1 : -1;
    }
};