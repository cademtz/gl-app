#include "dialog.hpp"
#include "app.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <memory>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <render/render2d_draw.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <util/defer.hpp>

namespace Dialog {

static const float GRID_SIZE = 25;
static const int NODE_HEIGHT = GRID_SIZE;
static const int NODE_WIDTH = GRID_SIZE*4;
static const float VIEWPORT_SCALE_MAX = 8.f;
static const float VIEWPORT_SCALE_MIN = 1.f/8;
glm::vec2 viewport_origin{0.f};
float viewport_scale = 1;

static glm::vec2 mclick_world{0.f}; // The last mouse click in world coords
static glm::vec2 eclick_world{0.f}; // The last clicked entity's world coords
static glm::vec2 viewport_pos{0.f}, viewport_size{0.f};

struct Span {
    std::string text;
};

struct Node {
    std::string name;
    std::string desc;
    std::vector<Span> spans;
    glm::vec2 pos{0.f};
};

using NodePtr = std::weak_ptr<Node>;

struct Graph {
    NodePtr root;
    std::vector<std::shared_ptr<Node>> nodes;
    float next_y = 0;
    uint32_t next_name = 0;

    NodePtr AddNode() {
        nodes.emplace_back(std::make_shared<Node>());
        std::shared_ptr<Node> node = nodes.back();
        
        node->name = std::to_string(next_name);
        ++next_name;

        node->pos.y = next_y;
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

void DrawNode(Render2d::Draw& draw, std::shared_ptr<Node> node);
glm::mat3 ViewToWorld();
glm::mat3 WorldToView();

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
                    viewport_origin = graph.nodes[i]->pos;
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
    ImGui::SliderFloat("##zoom", &viewport_scale, VIEWPORT_SCALE_MIN, VIEWPORT_SCALE_MAX, "%.3f", ImGuiSliderFlags_Logarithmic);

    ImVec2 win_pos = ImGui::GetWindowPos();
    ImVec2 win_size = ImGui::GetWindowSize();
    viewport_pos = glm::vec2{win_pos.x, win_pos.y};
    viewport_size = glm::vec2{win_size.x, win_size.y};

    if (!viewport_input)
        return;
    ImGuiIO& io = ImGui::GetIO();

    std::shared_ptr<Node> hovered_node;
    glm::mat3 to_world = ViewToWorld();
    glm::vec2 mouse_world = glm::vec2{to_world * glm::vec3{io.MousePos.x, io.MousePos.y, 1.f}};
    for (std::shared_ptr<Node> node : graph.nodes) {
        int left = node->pos.x, top = node->pos.y;
        int right = left + NODE_WIDTH, bottom = top + NODE_HEIGHT;
        if (mouse_world.x >= left && mouse_world.x <= right && mouse_world.y >= top && mouse_world.y <= bottom) {
            hovered_node = node;
            break;
        }
    }
    
    // Only handle one mouse button in a frame
    int mbtn_down = -1;
    for (int i = ImGuiMouseButton_COUNT-1; i >= 0; --i) {
        if (io.MouseDown[i])
            mbtn_down = i;
    }

    if (io.MouseClicked[ImGuiMouseButton_Left]) {
        selected_node_ptr = hovered_node;
        if (hovered_node) {
            mclick_world = mouse_world;
            eclick_world = hovered_node->pos;
        }
    } else if (mbtn_down == -1 && io.MouseWheel != 0) {
        float factor = glm::pow(2, (io.MouseWheel + 1)/8.f);
        if (io.MouseWheel < 0)
            factor = 1.f/glm::pow(2, (-io.MouseWheel + 1)/8.f);
        viewport_scale = glm::clamp(viewport_scale * factor, VIEWPORT_SCALE_MIN, VIEWPORT_SCALE_MAX);
    } 
    
    std::shared_ptr<Node> selected_node = selected_node_ptr.lock();
    if (selected_node != nullptr) {
        if (mbtn_down == ImGuiMouseButton_Left) {
            selected_node->pos = glm::floor(eclick_world + mouse_world - mclick_world + GRID_SIZE/2);
            selected_node->pos.x -= (int32_t)selected_node->pos.x % (int32_t)GRID_SIZE;
            selected_node->pos.y -= (int32_t)selected_node->pos.y % (int32_t)GRID_SIZE;
        }
    }

    if (mbtn_down == ImGuiMouseButton_Right || mbtn_down == ImGuiMouseButton_Middle) {
        viewport_origin -= glm::vec2{io.MouseDelta.x, io.MouseDelta.y} / viewport_scale;
    }
}

glm::mat3 WorldToView() {
    glm::vec2 center = {viewport_pos.x + (int32_t)viewport_size.x/2, viewport_pos.y + (int32_t)viewport_size.y/2};
    return glm::mat3{
        1.f, 0.f, 0.f,
        0.f, 1.f, 0.f,
        center.x - viewport_origin.x*viewport_scale, center.y - viewport_origin.y*viewport_scale, 1.f
    } * glm::mat3{
        viewport_scale, 0.f, 0.f,
        0.f, viewport_scale, 0.f,
        0.f, 0.f, 1.f
    };
}
glm::mat3 ViewToWorld() {
    glm::vec2 center = {viewport_pos.x + (int32_t)viewport_size.x/2, viewport_pos.y + (int32_t)viewport_size.y/2};
    return glm::mat3{
        1.f/viewport_scale, 0.f, 0.f,
        0.f, 1.f/viewport_scale, 0.f,
        0.f, 0.f, 1.f
    } * glm::mat3{
        1.f, 0.f, 0.f,
        0.f, 1.f, 0.f,
        -center.x + viewport_origin.x*viewport_scale, -center.y + viewport_origin.y*viewport_scale, 1.f
    };
}

void OnDrawGui(Render2d::Draw& draw) {
    // Draw grid
    int width, height;
    Platform::GetFrameBufferSize(&width, &height);

    draw.PushClip(glm::vec4{viewport_pos, viewport_size});
    defer { draw.PopClip(); };
    draw.PushTransform(WorldToView());
    defer { draw.PopTransform(); };

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
}

void DrawNode(Render2d::Draw& draw, std::shared_ptr<Node> node) {
    draw.SetColor(1,1,1);
    draw.Rect(node->pos.x, node->pos.y, NODE_WIDTH, NODE_HEIGHT);

    glm::vec4 color = {0.f, 0.f, 0.f, 1.f};
    if (node == selected_node_ptr.lock()) {
        color = {1.f, 0.f, 0.f, 1.f};
    }
    draw.SetColor(color);
    draw.TextAscii(App::font_default, node->pos, node->name);
}

void OnInput() {

}

}