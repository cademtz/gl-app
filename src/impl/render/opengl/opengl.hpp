#pragma once

#ifdef __EMSCRIPTEN__
#include "em_includes.hpp"
#else
#include "default_includes.hpp"
#endif


#if _IMPL_WINDOW == _IMPL_WINDOW_GLFW
#include <impl/window/glfw/glfw.hpp>
#endif