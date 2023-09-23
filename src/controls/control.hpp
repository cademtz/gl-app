#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include <input/inputhandler.hpp>
#include <glm/vec2.hpp>
#include <render/gui/draw.hpp>

namespace controls {

/**
 * @brief Handle input and describe common control properties and behaviors.
 * Warning: Use the `RunEvent(hid::Event, int32_t, int32_t)` function to run inputs.
 *  Using other input functions will bypass the default behavior (inputs won't run on children),
 *  and inputs will not be made relative to the child.
 */
class Control : public hid::InputHandler {
public:
    using Ptr = std::shared_ptr<Control>;
    using ConstPtr = std::shared_ptr<const Control>;
    using Size = glm::vec<2, uint32_t>;

    Control(Size layout_size);

    Size GetLayoutSize() const { return m_layout_size; }
    void ClearDrawFlag() { m_update_draw = false; }
    void ClearLayoutFlag() { m_update_layout = false; }

    /**
     * @brief Draw the control, in screen-space, at the specified `x` and `y` position.
     * This function calls @ref DrawImpl to perform drawing for it.
     * @see DrawImpl
     */
    void Draw(gui::Draw& draw, int32_t x, int32_t y);

    /**
     * @brief Determine if input should also be passed to controls underneath.
     * Typically ran after a call to an input handling function.
     * @return `true` if the received input should be passed to the next control underneath.
     */
    virtual bool ShouldPassThrough() const { return false;}

    /**
     * @brief Determine if the control, at this moment, should be drawn unconditionally.
     * The default function checks for child controls that should redraw.
     * @return `true` if the control should be drawn
     */
    virtual bool ShouldRedraw() const;
    void SetShouldRedraw();

    /**
     * @brief Whether parent controls should re-calculate layout.
     * The default function checks for child controls that should update.
     * @return `true` if parent controls should re-calculate layout
     */
    virtual bool ShouldUpdateLayout() const;

    /**
     * @brief Provide a potential layout size.
     * The control may accept, modify, or ignore the new size.
     */
    virtual void SuggestNewLayoutSize(Size new_size) { SetLayoutSize(new_size); }

    /**
     * @brief Notify the control that its parent's layout has been re-calculated.
     */
    void OnUpdateLayout();

    /**
     * @brief Run the event, but with all coordinates translated relative to `x` and `y`
     * @param x Control's X position
     * @param y Control's Y position
     */
    void RunEvent(const hid::Event& event, int32_t x, int32_t y);
    void RunEvent(const hid::Event& event) override;

protected:
    /**
     * @brief Draw the control in screen-space, at the specified `x` and `y` position.
     * This function is typically called by @ref Draw
     * @see Draw
     */
    virtual void DrawImpl(gui::Draw& draw, int32_t x, int32_t y) = 0;

    /**
     * @brief Set the layout size and indicate layout to be updated
     */
    void SetLayoutSize(Size size);

    void AddChild(Control::Ptr child);
    
    std::vector<Control::Ptr> m_children;

private:
    Size m_layout_size;

    // `true` if layout should be re-calculated
    bool m_update_layout = true;
    // `true` if control should be drawn
    bool m_update_draw = true;
};

}