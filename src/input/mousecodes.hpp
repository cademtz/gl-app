#pragma once
#include <cstdint>

namespace hid {

enum MouseAction : uint8_t {
    MOUSE_ACTION_PRESS = 0,
    MOUSE_ACTION_RELEASE = 1,
};

enum MouseCode : uint8_t {
    MOUSE_UNKNOWN = 0,
    MOUSE_LEFT = 1,
    MOUSE_RIGHT = 2,
    MOUSE_MIDDLE = 3,
};

}