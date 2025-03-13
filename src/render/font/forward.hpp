#pragma once
#include <memory>
#include <cstdint>

using codepoint_t = uint32_t;

class FontAtlas;
class _FontHandle;
/** A lightweight font handle. Passing by value is recommended. */
using FontHandle = std::shared_ptr<_FontHandle>;
