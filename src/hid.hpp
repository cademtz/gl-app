#pragma once
#include "keycodes.hpp"
#include "mousecodes.hpp"

namespace hid {

/**
 * @brief State of a keyboard's key
 */
struct PhysicalKey {
    KeyCode code;
    KeyAction action;
};

/**
 * @brief State of a mouse's button
 */
struct MouseButton {
    MouseAction action;
    MouseCode code;
};

/**
 * @brief A mouse position in screen coordinates
 */
struct MousePos {
    double x, y;
};

/**
 * @brief Scrolling motion
 */
struct Scroll {
    double x_off;
    /**
     * @brief Positive for upwards motion
     */
    double y_off;
};

/**
 * @brief State of a single touch input, in screen coordinates
 */
struct SingleTouch {
    double x, y;
    /**
     * @brief Pressure percentage, between 0 and 1.
     * 0 indicates the surface is not being touched.
     */
    double pressure;
    /**
     * @brief Rotation in radians. Must be 0 if no rotation is detected.
     */
    double rotate;  
};

/**
 * @brief State of multiple simultaneous touch inputs, in screen coordinates
 */
struct MultiTouch {
    double x[8], y[8];
    uint8_t num_touches;
};

}