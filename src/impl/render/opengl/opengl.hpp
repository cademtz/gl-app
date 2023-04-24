#pragma once

#ifdef __EMSCRIPTEN__
#include "em_includes.hpp"
#else
#include "default_includes.hpp"
#endif

#define _IMPL_GLSL_VERSION "300 es"
#define _IMPL_GLSL_VERSION_HEADER "#version " _IMPL_GLSL_VERSION "\n"