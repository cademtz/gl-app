#pragma once

#ifdef __EMSCRIPTEN__
#include "em_includes.hpp"
#else
#include "default_includes.hpp"
#endif

#define IMPL_GLSL_VERSION "300 es"
#define IMPL_GLSL_VERSION_HEADER "#version " IMPL_GLSL_VERSION "\n"