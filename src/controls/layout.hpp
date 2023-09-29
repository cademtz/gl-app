#pragma once
#include <vector>
#include <memory>
#include <unordered_set>
#include <stdint.h>
#include <glm/glm.hpp>
#include <input/inputhandler.hpp>

namespace gui { class Draw; }

namespace widgets {

/**
 * @brief Layout behavior flags.
 * Conflicting flags are ignored.
 */
namespace LayoutFlag {
    /** Vertical arrangement. If not set, horizontal is used. */
    const uint16_t VERTICAL = 1 << 0;
    /** Fill horizontal space */
    const uint16_t H_FILL = 1 << 1;
    /** Fill vertical space */
    const uint16_t V_FILL = 1 << 2;
    /** Left-justify contents */
    const uint16_t L_JUSTIFY = 1 << 3;
    /** Right-justify contents */
    const uint16_t R_JUSTIFY = 1 << 4;
    /** Top-justify contents */
    const uint16_t T_JUSTIFY = 1 << 5;
    /** Bottom-justify contents */
    const uint16_t B_JUSTIFY = 1 << 6;
};

/**
 * @brief Base widget and layout node.
 * A layout rect may extend outside of its parent, but only to the bottom/right.
 * Use the @ref 
 */
class Widget : public hid::InputHandler {
public:
    using Size = glm::vec<2, uint16_t>;
    using Rect = glm::vec<4, uint16_t>;
    using Ptr = std::shared_ptr<Widget>;
    using ConstPtr = std::shared_ptr<const Widget>;

    ~Widget();
    /** Add a new child anywhere in the list */
    void InsertChild(Widget::Ptr child, size_t index);
    /** Add a new child at end of list. Shortcut for @ref InsertChild */
    void AddChild(Widget::Ptr child) {
        InsertChild(child, m_children.size());
    }
    void RemoveChild(Widget::Ptr child);

    /** Draw the control and validate the drawing */
    void Redraw(gui::Draw& draw);
    /**
     * @brief Layout all children and update the current content size.
     * This validates the current and child layouts.
     */
    void CalcLayout();
    /** If false, one or more child rects must be recalculated (but not its own) */
    bool IsValidLayout() const { return m_valid_layout; }
    /** If false, the widget must be redrawn */
    bool IsValidDrawing() const { return m_valid_drawing; }
    /** @return true if the layout rect size is 0 */
    bool IsHidden() const { return m_layoutrect[2] == 0 || m_layoutrect[3] == 0; }

    const std::vector<Widget::Ptr>& Children() { return m_children; }
    const std::vector<Widget::ConstPtr>& Children() const {
        return reinterpret_cast<const std::vector<Widget::ConstPtr>&>(m_children);
    }
    /** @return The default or minimum size */
    Size MinSize() const { return m_minsize; }
    /** @return The maxmimum size. Only used if H_FILL or V_FILL is set. */
    Size MaxSize() const { return m_maxsize; }
    /** @return Combined values from @ref LayoutFlag */
    uint16_t LayoutFlags() const { return m_layoutflags; }
    /** @return The final rect, `{x,y,w,h}`, relative to the root */
    const Rect& LayoutRect() const { return m_layoutrect; }
    /** @return Size of contents, unclipped. Useful for scrollbars. */
    Size ContentSize() const { return m_contentsize; }
    
    Widget& SetMinSize(Size size) {
        m_minsize = size;
        return *this;
    }
    Widget& SetMaxSize(Size size) {
        m_maxsize = size;
        return *this;
    }
    /** Replace the current flags */
    Widget& SetLayoutFlags(uint16_t flags);
    Widget& SetLayoutRect(const Rect& r);

protected:
    /** Draw the widget over its layout rect */
    virtual void Draw(gui::Draw& draw) {}

private:
    /**
     * @brief Raw pointer to the parent.
     * Before a parent is destroyed, it must clear this value for all children.
     */
    Widget* m_parent;
    std::vector<Widget::Ptr> m_children;

    Size m_minsize{0};
    Size m_maxsize{(uint16_t)-1};
    /** Padding, `{l,t,r,b}` */
    Rect m_padding{0};
    Rect m_layoutrect{0};
    Size m_contentsize{0};
    uint16_t m_layoutflags = 0;

    bool m_valid_layout = false;
    bool m_valid_drawing = false;

    void InvalidateLayout();
    void InvalidateDrawing();

    /**
     * Find the smallest growable child's size and the additional size needed to equal the next smallest growable child
     * @param dimension 0 for width, 1 for height
     * @param out_target_size Layouts with this size should be grown next
     * @param out_give Additional size for the target layouts to reach the next target size
     * @return Number of target_size layouts that want more space
     */
    size_t CalcChildHunger(size_t dimension, uint16_t* out_target_size, uint16_t* out_give) const;
};

}