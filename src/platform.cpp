#include "platform.hpp"
#include "app.hpp"
#include <list>
#include <cstdio>
#include <cassert>

namespace Platform {

static bool should_close = false;
static std::list<RepeatTaskCallback> repeat_tasks;

void Warning(const char* Msg, const char* File, int Line)
{
    fprintf(stdout, "[!] %s\n", Msg);
    if (File)
        fprintf(stdout, "\t(file \"%s\", line %d)\n", File, Line);
}

void Error(const char* Msg, const char* File, int Line)
{
    fprintf(stderr, "[!] %s\n", Msg);
    if (File)
        fprintf(stderr, "\t(file \"%s\", line %d)\n", File, Line);

    assert(0 && "Error called");
}

void SetShouldClose() { should_close = true; }
bool ShouldClose() { return should_close; }

void AddRepeatingTask(RepeatTaskCallback task) {
    repeat_tasks.push_back(task);
}

bool RunTasksOnce()
{
    if (ShouldClose())
        return false;
    
    auto it = repeat_tasks.begin();
    while (it != repeat_tasks.end()) {
        RepeatTaskCallback callback = *it;
        bool result = callback();
        if (!result)
            it = repeat_tasks.erase(it);
        else
            ++it;
    }

    return !repeat_tasks.empty();
}

void Exit() {
    SetShouldClose();
    App::OnCleanup();
    Cleanup();
}

}