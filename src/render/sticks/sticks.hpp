#pragma once
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace sticks {

enum class SegmentCap {
    NONE,
    SQUARE,
    CIRCLE,
};

struct Point {
    glm::vec2 pos;
    SegmentCap cap;
    float radius;
    glm::vec4 rgba;
};

}