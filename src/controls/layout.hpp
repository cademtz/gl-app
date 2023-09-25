#pragma once
#include <stdint.h>
#include <glm/glm.hpp>
#include <vector>

namespace controls {

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
 * @brief Layout node.
 * The final rectangle may extend outside of its parent.
 * An item can only extend to the left/bottom of its parent.
 */
class Layout {
public:
    using Size = glm::vec<2, uint16_t>;
    using Rect = glm::vec<4, uint16_t>;

    /** @return true if the final rectangle size is 0 */
    bool IsHidden() const { return rect[2] == 0 || rect[3] == 0; }
    /** Calculate @ref contentsize and the children's @ref rect and @ref contentsize values */
    void Calculate();
    const std::vector<Layout*>& Children() const { return children; }
    std::vector<Layout*>& Children() { return children; }
    /** @return The default or minimum size */
    Size MinSize() const { return minsize; }
    /** @return The maxmimum size. Only used if H_FILL or V_FILL is set. */
    Size MaxSize() const { return maxsize; }
    /** @return Combined values from @ref LayoutFlag */
    uint16_t Flags() const { return flags; }
    /** @return The final rect, `{x,y,w,h}`, relative to the root */
    const Rect& Xywh() const { return rect; }
    
    Layout& SetMinSize(Size size) {
        minsize = size;
        return *this;
    }
    Layout& SetMaxSize(Size size) {
        maxsize = size;
        return *this;
    }
    /** Replace the current flags */
    Layout& SetFlags(uint16_t flags) {
        this->flags = flags;
        return *this;
    }
    Layout& SetRect(const Rect& r) {
        rect = r;
        return *this;
    }

private:
    Size minsize{0};
    Size maxsize{(uint16_t)-1};
    /** Margins, `{l,t,r,b}` */
    Rect margins{0};
    Rect rect{0};
    /** Size of contents, unclipped. Useful for scrollbars */
    Size contentsize{0};
    uint16_t flags = 0;
    std::vector<Layout*> children;
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