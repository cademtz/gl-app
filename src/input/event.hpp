#include "hid.hpp"
#include <variant>

namespace hid {
using _EventVariant = std::variant<
    MouseButton,
    MousePos,
    Scroll,
    PhysicalKey,
    CharacterKey,
    SingleTouch,
    MultiTouch
>;

/**
 * @brief Any signal from an input device
 */
class Event : public _EventVariant {
public:
    template <class T>
    Event(const T& value) : _EventVariant(value) {}
    /** @return A new event with the X and Y positions offset */
    inline Event Translate(int32_t x, int32_t y) const;
};

/**
 * @brief Offset the X and Y positions (if any) of an event.
 * Use Event::Translate. It's much more convenient.
 */
struct EventTranslator {
    int32_t x_off, y_off;
    
    Event operator()(const MouseButton& value)   { return value; }
    Event operator()(const Scroll& value)        { return value; }
    Event operator()(const PhysicalKey& value)   { return value; }
    Event operator()(const CharacterKey& value)  { return value; }

    Event operator()(const MousePos& value) {
        MousePos clone = value;
        clone.x += x_off, clone.y += y_off;
        return clone;
    }
    Event operator()(const SingleTouch& value) {
        SingleTouch clone = value;
        clone.x += x_off, clone.y += y_off;
        return clone;
    }
    Event operator()(const MultiTouch& value) {
        MultiTouch clone = value;
        for (uint8_t i = 0; i < clone.num_touches; ++i)
            clone.x[i] += x_off, clone.y[i] += y_off;
        return clone;
    }
};

Event Event::Translate(int32_t x, int32_t y) const {
    return std::visit(EventTranslator{x, y}, *this);
}

}