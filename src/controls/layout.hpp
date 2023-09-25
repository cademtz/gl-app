#pragma once
#include <stdint.h>
#include <glm/glm.hpp>
#include <vector>

namespace controls {

/**
 * @brief Layout behavior
 * Certain combinations will create conflicts.
 */
enum class LayoutFlag : uint16_t {
    /** Vertical arrangement. If not set, horizontal is used. */
    VERTICAL = 1 << 1,
    /** Fill horizontal space */
    H_FILL = 1 << 2,
    /** Fill vertical space */
    V_FILL = 1 << 3,
    /** Left-justify contents */
    L_JUSTIFY = 1 << 4,
    /** Right-justify contents */
    R_JUSTIFY = 1 << 5,
    /** Top-justify contents */
    T_JUSTIFY = 1 << 6,
    /** Bottom-justify contents */
    B_JUSTIFY = 1 << 7,
};

/**
 * @brief Layout node.
 * The final rectangle may extend outside of its parent.
 * An item can only extend to the left/bottom of its parent.
 */
struct Layout {
    using Id = uint16_t;
    using Size = glm::vec<2, uint16_t>;
    using Rect = glm::vec<4, uint16_t>;

    /** The minimum and default size. */
    Size minsize{0};
    /** Maximum size. Only used if H_FILL or V_FILL is set. */
    Size maxsize{(uint16_t)-1};
    /** Margins, `{l,t,r,b}` */
    Rect margins{0};
    /** The final rect, `{x,y,w,h}`, clipped and relative to the root */
    Rect rect{0};
    /** Size of contents, unclipped. Useful for scrollbars */
    Size contentsize{0};
    /** Combined values from @ref LayoutFlag */
    uint16_t flags = 0;
    std::vector<Layout*> children;

    uint16_t GetPreferredSize(size_t dimension) const;
    /** @return true if the final rectangle size is 0 */
    bool IsHidden() const { return rect[2] == 0 || rect[3] == 0; }
    /** Calculate @ref contentsize and the children's @ref rect and @ref contentsize values */
    void Calculate();

private:
    /**
     * Find the smallest growable child's size and the additional size needed to equal the next smallest growable child
     * @param dimension 0 for width, 1 for height
     * @param out_target_size Layouts with this size should be grown next
     * @param out_give Additional size for the target layouts to reach the next target size
     * @return Number of layouts that want more space
     */
    size_t CalcChildHunger(size_t dimension, uint16_t* out_target_size, uint16_t* out_give) const;
};

}