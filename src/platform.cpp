#include "platform.hpp"
#include "app.hpp"
#include <list>
#include <cstdio>
#include <cassert>
#include <string_view>

namespace Platform {

static bool should_close = false;
static std::list<RepeatTaskCallback> repeat_tasks;

void Warning(std::string_view msg, const char* file, int line) {
    fprintf(stdout, "[!] %.*s\n", msg.length(), msg.data());
    if (file)
        fprintf(stdout, "\t(file \"%s\", line %d)\n", file, line);
}

void Error(std::string_view msg, const char* file, int line) {
    fprintf(stderr, "[!] %.*s\n", msg.length(), msg.length(), msg.data());
    if (file)
        fprintf(stderr, "\t(file \"%s\", line %d)\n", file, line);

    assert(0 && "Error called");
}

void SetShouldClose() { should_close = true; }
bool ShouldClose() { return should_close; }

void AddRepeatingTask(RepeatTaskCallback task) {
    repeat_tasks.push_back(task);
}

bool RunTasksOnce() {
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