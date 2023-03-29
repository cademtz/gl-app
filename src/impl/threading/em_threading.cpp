#include <emscripten.h>
#include <platform.hpp>
#include <list>

namespace Platform
{
    void RunTasksForever() {
        emscripten_set_main_loop(
            [] {
                if (!RunTasksOnce())
                    Exit();
            }, 0, true);
    }
}