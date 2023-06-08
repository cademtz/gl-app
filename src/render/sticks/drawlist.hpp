#pragma once
#include <vector>
#include <cstdint>
#include <glm/mat3x3.hpp>
#include <render/texture.hpp>

namespace sticks {

struct Vertex {
    float x, y;
    /**
     * @brief 
     */
    float u, v;
    float r, g, b, a;
    /**
     * @brief A value of `+1` and `-1` create convex and concave curves respectively
     */
    //float convex;
};

struct DrawCall {
    uint32_t index_offset = 0;
    uint32_t index_count = 0;
    glm::mat3x3 transform = glm::mat3x3(1.f);
};

struct DrawList {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<DrawCall> calls;
};

}