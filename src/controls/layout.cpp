#include "layout.hpp"

namespace controls {

static const uint16_t fill_mask[] = { LayoutFlag::H_FILL, LayoutFlag::V_FILL };
static const uint16_t justify_mask[] = {
    LayoutFlag::L_JUSTIFY | LayoutFlag::R_JUSTIFY,
    LayoutFlag::T_JUSTIFY | LayoutFlag::B_JUSTIFY
};

size_t Layout::CalcChildHunger(size_t dimension, uint16_t* out_target_size, uint16_t* out_give) const {
    uint16_t target_size = (uint16_t)-1;
    // Either the next smallest 
    uint16_t next_target_size = target_size;
    size_t count = 0;

    // Find smallest growable layout size (and number of them)
    for (Layout* child : children) {
        uint16_t size = child->rect[dimension + 2];

        // Can't grow
        if (!(child->flags & fill_mask[dimension]) || size >= child->maxsize[dimension])
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
        if (child->maxsize[dimension] < next_target_size)
            next_target_size = child->maxsize[dimension];
    }

    *out_target_size = target_size;
    *out_give = next_target_size - target_size;
    return count;
}

void Layout::Calculate() {
    
    glm::length_t dir = 0; // Direction. 0 for horizontal (also the x index)
    if (flags & LayoutFlag::VERTICAL)
        dir = 1;
    glm::length_t opp = dir ^ 1; // Perpendicular direction
    
    // First calculate minimum required content size
    // and set all children to their minimum size.

    size_t num_fills = 0;
    Size required = Size{0};
    for (Layout* child : children) {
        child->rect[2] = child->minsize[0];
        child->rect[3] = child->minsize[1];
        required[dir] += child->minsize[dir];
        // Store the largest required size (of opposite dimension)
        required[opp] = std::max(required[opp], child->minsize[opp]);

        // If the child can grow in opposite dimension, do so. They aren't sharing that space.
        if (child->flags & fill_mask[opp])
            child->rect[opp + 2] = std::min(rect[opp + 2], child->maxsize[opp]);
    }
    
    // Next, calculate the remaining space and distribute it to create the most evenly-sized fill layouts
    if (required[dir] <= rect[dir + 2]) {
        uint16_t remaining = rect[dir + 2] - required[dir];
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

            for (Layout* child : children) {
                if (child->rect[dir + 2] == target_size) {
                    child->rect[dir + 2] += give_size;
                    if (crumbs) {
                        ++child->rect[dir + 2];
                        --crumbs;
                    }
                }
            }
        }
    }
    
    // Finally, justify all children, write their new positions, and run their layout
    // TODO: Justify children
    uint16_t cursor = 0;
    for (Layout* child : children) {
        child->rect[dir] = rect[dir] + cursor;
        child->rect[opp] = rect[opp];
        cursor += child->rect[dir + 2];

        child->Calculate();
    }
}

}