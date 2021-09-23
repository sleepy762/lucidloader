#pragma once
#include <uefi.h>

inline void EchoCmd(char args[], char** currPathPtr)
{
    if (args) 
        printf("\n%s", args);
}

inline const char* EchoBrief(void)
{
    return "Print a string. Used for debugging.";
}

inline const char* EchoLong(void)
{
    return "Usage: echo <string>";
}
