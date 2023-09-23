#pragma once
#include <memory>

struct FontBakeConfig;

namespace gui {

class FontAtlas;
class _FontHandle;

/** A lightweight font handle. Passing by value is recommended. */
using FontHandle = std::shared_ptr<_FontHandle>;

/**
 * Create, store, and update all fonts used for GUI rendering.
 * Fonts are not updated or created immediately.
 * @ref Update must be called to run any pending requests.
 */
class FontManager {
public:
    /** Queue the creation of a font, get an immediate handle */
    static FontHandle CreateFont(FontBakeConfig&& config);
    /** @return A font atlas. May return `nullptr` if the atlas is not yet baked. */
    static const FontAtlas* GetAtlas(FontHandle handle);
    /** Run all pending tasks */
    static void RunQueue();
    /** Call this before the application exits. */
    static void Cleanup();
};

}