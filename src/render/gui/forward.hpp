#pragma once
#include <memory>

namespace gui {
    class FontAtlas;
    class _FontHandle;
    /** A lightweight font handle. Passing by value is recommended. */
    using FontHandle = std::shared_ptr<_FontHandle>;

    class Draw;
    struct DrawList;
}