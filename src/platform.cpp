#include "platform.hpp"
#include "app.hpp"
#include <list>
#include <cstdio>
#include <cassert>

static bool should_close = false;
static std::list<Platform::RepeatTaskCallback> repeat_tasks;

void Platform::Warning(const char* Msg, const char* File, int Line)
{
    fprintf(stdout, "[!] %s\n", Msg);
    if (File)
        fprintf(stdout, "\t(file \"%s\", line %d)\n", File, Line);
}

void Platform::Error(const char* Msg, const char* File, int Line)
{
    fprintf(stderr, "[!] %s\n", Msg);
    if (File)
        fprintf(stderr, "\t(file \"%s\", line %d)\n", File, Line);

    assert(0 && "Platform::Error called");
}

void Platform::SetShouldClose() { should_close = true; }
bool Platform::ShouldClose() { return should_close; }

void Platform::AddRepeatingTask(Platform::RepeatTaskCallback task) {
    repeat_tasks.push_back(task);
}

bool Platform::RunTasksOnce()
{
    if (ShouldClose())
        return false;
    
    auto it = repeat_tasks.begin();
    while (it != repeat_tasks.end()) {
        Platform::RepeatTaskCallback callback = *it;
        bool result = callback();
        if (!result)
            it = repeat_tasks.erase(it);
        else
            ++it;
    }

    return !repeat_tasks.empty();
}

void Platform::Exit() {
    SetShouldClose();
    App::OnCleanup();
    Cleanup();
}