#include "hid.hpp"
#include <variant>

namespace hid {
using _EventVariant = std::variant<
    hid::MouseButton,
    hid::MousePos,
    hid::Scroll,
    hid::PhysicalKey,
    hid::CharacterKey,
    hid::SingleTouch,
    hid::MultiTouch
>;

/**
 * @brief Any signal from an input device
 */
using Event = _EventVariant;

/**
 * @brief Offset the X and Y positions (if any) of an event
 */
struct EventTranslator {
    int32_t x_off, y_off;
    
    hid::Event operator()(const hid::MouseButton& value)   { return value; }
    hid::Event operator()(const hid::Scroll& value)        { return value; }
    hid::Event operator()(const hid::PhysicalKey& value)   { return value; }
    hid::Event operator()(const hid::CharacterKey& value)  { return value; }

    hid::Event operator()(const hid::MousePos& value) {
        hid::MousePos clone = value;
        clone.x += x_off, clone.y += y_off;
        return clone;
    }
    hid::Event operator()(const hid::SingleTouch& value) {
        hid::SingleTouch clone = value;
        clone.x += x_off, clone.y += y_off;
        return clone;
    }
    hid::Event operator()(const hid::MultiTouch& value) {
        hid::MultiTouch clone = value;
        for (uint8_t i = 0; i < clone.num_touches; ++i)
            clone.x[i] += x_off, clone.y[i] += y_off;
        return clone;
    }
};

}