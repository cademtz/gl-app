#include "layout.hpp"
#include <assert.h>

namespace widgets {

static const uint16_t fill_mask[] = { LayoutFlag::H_FILL, LayoutFlag::V_FILL };
static const uint16_t justify_mask[] = {
    LayoutFlag::L_JUSTIFY | LayoutFlag::R_JUSTIFY,
    LayoutFlag::T_JUSTIFY | LayoutFlag::B_JUSTIFY
};

Widget::~Widget() {
    for (Widget::Ptr child : m_children)
        child->m_parent = nullptr;
}

void Widget::InsertChild(Widget::Ptr child, size_t index) {
    assert(child->m_parent != this && "Do not add the same child multiple times");
    if (child->m_parent == this)
        return;
    
    if (child->m_parent)
        child->m_parent->RemoveChild(child);
    child->m_parent = this;
    m_children.insert(m_children.begin() + index, child);
}

void Widget::RemoveChild(Widget::Ptr child) {
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it == m_children.end())
        return;
    (*it)->m_parent = nullptr;
    m_children.erase(it);
}

Widget& Widget::SetLayoutFlags(uint16_t flags) {
    m_layoutflags = flags;
    InvalidateLayout();
    return *this;
}

Widget& Widget::SetLayoutRect(const Rect& r) {
    m_layoutrect = r;
    InvalidateLayout();
    return *this;
}

void Widget::InvalidateLayout() {
    m_valid_layout = false;
    InvalidateDrawing();
}
void Widget::InvalidateDrawing() {
    m_valid_drawing = false;
}

void Widget::Redraw(gui::Draw& draw) {
    Draw(draw);
    m_valid_drawing = true;
}

size_t Widget::CalcChildHunger(size_t dimension, uint16_t* out_target_size, uint16_t* out_give) const {
    uint16_t target_size = (uint16_t)-1;
    // Either the next smallest 
    uint16_t next_target_size = target_size;
    size_t count = 0;

    // Find smallest growable layout size (and number of them)
    for (Widget::Ptr child : m_children) {
        uint16_t size = child->m_layoutrect[dimension + 2];

        // Can't grow
        if (!(child->m_layoutflags & fill_mask[dimension]) || size >= child->m_maxsize[dimension])
            continue;

        // Not the smallest. But it may be a good next target.
        if (size > target_size) {
            if (size < next_target_size)
                next_target_size = size;
            continue;
        }

        if (size < target_size) {
            count = 0;
            target_size = size;
        }
        ++count;

        // Its maximum may be a good next target 
        if (child->m_maxsize[dimension] < next_target_size)
            next_target_size = child->m_maxsize[dimension];
    }

    *out_target_size = target_size;
    *out_give = next_target_size - target_size;
    return count;
}

void Widget::CalcLayout() {
    
    glm::length_t dir = 0; // Direction. 0 for horizontal (also the x index)
    if (m_layoutflags & LayoutFlag::VERTICAL)
        dir = 1;
    glm::length_t opp = dir ^ 1; // Perpendicular direction
    
    // First calculate minimum required content size
    // and set all children to their minimum size.

    size_t num_fills = 0;
    Size required = Size{0};
    for (Widget::Ptr child : m_children) {
        child->m_layoutrect[2] = child->m_minsize[0];
        child->m_layoutrect[3] = child->m_minsize[1];
        required[dir] += child->m_minsize[dir];
        // Store the largest required size (of opposite dimension)
        required[opp] = std::max(required[opp], child->m_minsize[opp]);

        // If the child can grow in opposite dimension, do so. They aren't sharing that space.
        if (child->m_layoutflags & fill_mask[opp])
            child->m_layoutrect[opp + 2] = std::min(m_layoutrect[opp + 2], child->m_maxsize[opp]);
    }
    
    // Next, calculate the remaining space and distribute it to create the most evenly-sized fill layouts
    if (required[dir] <= m_layoutrect[dir + 2]) {
        uint16_t remaining = m_layoutrect[dir + 2] - required[dir];
        uint16_t target_size, give_size;
        size_t hungry;
        while (remaining && (hungry = CalcChildHunger(dir, &target_size, &give_size))) {
            uint64_t total_give = (uint64_t)give_size * hungry;
            int16_t crumbs = 0;
            if (total_give > remaining) {
                give_size = (uint16_t)(remaining / hungry);
                crumbs = (uint16_t)(remaining % hungry);
            }

            remaining -= (uint16_t)(give_size * hungry + crumbs);

            for (Widget::Ptr child : m_children) {
                if (child->m_layoutrect[dir + 2] == target_size) {
                    child->m_layoutrect[dir + 2] += give_size;
                    if (crumbs) {
                        ++child->m_layoutrect[dir + 2];
                        --crumbs;
                    }
                }
            }
        }

        m_contentsize = required;
    }
    
    // Finally, justify all children, write their new positions, and run their layout
    // TODO: Justify children
    uint16_t cursor = 0;
    for (Widget::Ptr child : m_children) {
        child->m_layoutrect[dir] = m_layoutrect[dir] + cursor;
        child->m_layoutrect[opp] = m_layoutrect[opp];
        cursor += child->m_layoutrect[dir + 2];

        child->CalcLayout();
    }

    m_valid_layout = true;
}

}