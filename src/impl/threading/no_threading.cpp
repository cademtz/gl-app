#include <platform.hpp>

namespace Platform
{
    void RunTasksForever()
    {
        while (RunTasksOnce())
            ;
    }
}