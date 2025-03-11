#include "dialog.hpp"
#include "app.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <memory>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <render/gui/draw.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Dialog {

static const float GRID_SIZE = 25;
static const int NODE_HEIGHT = GRID_SIZE;
static const int NODE_WIDTH = GRID_SIZE*4;
glm::vec2 viewport_origin{0.f};
float viewport_scale = 1;

struct Span {
    std::string text;
};

struct Node {
    std::string name;
    std::string desc;
    std::vector<Span> spans;
    float x = 0, y = 0;
};

using NodePtr = std::weak_ptr<Node>;

struct Graph {
    NodePtr root;
    std::vector<std::shared_ptr<Node>> nodes;
    float next_y = GRID_SIZE;
    uint32_t next_name = 0;

    NodePtr AddNode() {
        nodes.emplace_back(std::make_shared<Node>());
        std::shared_ptr<Node> node = nodes.back();
        
        node->name = std::to_string(next_name);
        ++next_name;

        node->x = GRID_SIZE;
        node->y = next_y;
        next_y += GRID_SIZE;
        return node;
    }

    bool RemoveNode(NodePtr node_ptr) {
        std::shared_ptr<Node> node = node_ptr.lock();
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (node == nodes[i]) {
                nodes.erase(nodes.begin() + i);
                return true;
            }
        }
        return false;
    }
};

static Graph graph;
static NodePtr selected_node_ptr;

void OnSetup() {
    graph.AddNode();
    graph.AddNode();
    graph.AddNode();
}

void OnImGuiDock() {
    ImGui::DockBuilderDockWindow("Dialog graph", App::imgui_dock_left);
}

void OnImGui() {
    if (ImGui::Begin("Dialog graph")) {
        std::shared_ptr<Node> selected_node = selected_node_ptr.lock();

        if (ImGui::BeginListBox("Nodes")) {
            for (size_t i = 0; i < graph.nodes.size(); ++i) {
                ImGui::PushID(i);
                if (ImGui::Selectable(graph.nodes[i]->name.c_str(), graph.nodes[i] == selected_node)) {
                    selected_node_ptr = graph.nodes[i];
                }
                ImGui::PopID();
            }
            ImGui::EndListBox();
        }

        if (selected_node != nullptr) {
            ImGui::InputText("Name", &selected_node->name);
            ImGui::InputTextMultiline("Desc", &selected_node->desc);
        }
    }
    ImGui::End();
}

void OnImGuiViewport(bool viewport_input) {
    ImGui::Text("Zoom"); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    ImGui::SliderFloat("##zoom", &viewport_scale, 1.f/4, 4, "%.3f", ImGuiSliderFlags_Logarithmic);

    if (!viewport_input)
        return;
    ImGuiIO& io = ImGui::GetIO();

    std::shared_ptr<Node> hovered_node;
    for (std::shared_ptr<Node> node : graph.nodes) {
        int left = node->x, top = node->y;
        int right = left + NODE_WIDTH, bottom = top + NODE_HEIGHT;
        if (io.MousePos.x >= left && io.MousePos.x <= right && io.MousePos.y >= top && io.MousePos.y <= bottom) {
            hovered_node = node;
            break;
        }
    }
    
    if (io.MouseClicked[0]) {
        selected_node_ptr = hovered_node;
    }
    
    std::shared_ptr<Node> selected_node = selected_node_ptr.lock();
    if (selected_node != nullptr) {
        if (io.MouseDown[0]) {
            selected_node->x += io.MouseDelta.x;
            selected_node->y += io.MouseDelta.y;
        } else {
            selected_node->x = (int32_t)selected_node->x - ((int32_t)(selected_node->x-(GRID_SIZE/2)) % (int32_t)GRID_SIZE)+(int32_t)(GRID_SIZE/2);
            selected_node->y = (int32_t)selected_node->y - ((int32_t)(selected_node->y-(GRID_SIZE/2)) % (int32_t)GRID_SIZE)+(int32_t)(GRID_SIZE/2);
        }
    }
}

void DrawNode(gui::Draw& draw, std::shared_ptr<Node> node);
glm::vec2 ViewToWorld(glm::vec2 pos) {
    ImVec2 win_pos = ImGui::GetWindowPos();
    ImVec2 win_size = ImGui::GetWindowSize();
    glm::vec2 win_center{win_pos.x + win_size.x/2, win_pos.y + win_size.y/2};
    return (pos - win_center)/viewport_scale + viewport_origin;
}
glm::vec2 WorldToView(glm::vec2 pos) {
    ImVec2 win_pos = ImGui::GetWindowPos();
    ImVec2 win_size = ImGui::GetWindowSize();
    glm::vec2 win_center{win_pos.x + win_size.x/2, win_pos.y + win_size.y/2};
    return (pos - viewport_origin)*viewport_scale + win_center;
}

void OnDrawGui(gui::Draw& draw) {
    // Draw grid
    int width, height;
    Platform::GetFrameBufferSize(&width, &height);

    draw.PushTransform(glm::mat3{
        1.f, 0.f, 0.f,
        0.f, 1.f, 0.f,
        (float)width/2 + viewport_origin.x, (float)height/2 + viewport_origin.y, 1.f
    } * glm::mat3{
        viewport_scale, 0.f, 0.f,
        0.f, viewport_scale, 0.f,
        0.f, 0.f, 1.f
    });

    for (int x = 0; x*GRID_SIZE < width; ++x) {
        for (int y = 0; y*GRID_SIZE < height; ++y) {
            float value = 0.2f;
            if ((x&1) == (y&1)) {
                value = 0.15f;
            }
            draw.SetColor(value, value, value);
            draw.Rect(x*GRID_SIZE, y*GRID_SIZE, GRID_SIZE, GRID_SIZE);
        }
    }

    std::shared_ptr<Node> selected_node = selected_node_ptr.lock();
    
    for (size_t i = graph.nodes.size(); i > 0; --i) {
        if (graph.nodes[i-1] == selected_node)
            continue; // Save this for last
        DrawNode(draw, graph.nodes[i-1]);
    }
    if (selected_node != nullptr)
        DrawNode(draw, selected_node);
    
    draw.PopTransform();
}

void DrawNode(gui::Draw& draw, std::shared_ptr<Node> node) {
    draw.SetColor(1,1,1);
    draw.Rect(node->x, node->y, NODE_WIDTH, NODE_HEIGHT);

    glm::vec4 color = {0.f, 0.f, 0.f, 1.f};
    if (node == selected_node_ptr.lock()) {
        color = {1.f, 0.f, 0.f, 1.f};
    }
    draw.SetColor(color);
    draw.TextAscii(App::font_default, glm::vec2{node->x, node->y}, node->name);
}

void OnInput() {

}

}