#include "platform.hpp"
#include <stdio.h>
#include <assert.h>

void Platform::Warning(const char* Msg, const char* File, int Line)
{
    fprintf(stdout, "[!] %s\n", Msg);
    if (File)
        fprintf(stdout, "\t(file \"%s\", line %d)", File, Line);
}

void Platform::Error(const char* Msg, const char* File, int Line)
{
    fprintf(stderr, "[!] %s\n", Msg);
    if (File)
        fprintf(stderr, "\t(file \"%s\", line %d)", File, Line);
    
    assert(0 && "Platform::Error called");
}
