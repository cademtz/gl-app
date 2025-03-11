#pragma once

namespace impl::window {

void setup(int width, int height);
void cleanup();
void PreRender();
void PostRender();
}