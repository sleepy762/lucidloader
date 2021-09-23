#pragma once
#include <uefi.h>

inline void PwdCmd(char args[], char** currPathPtr)
{
    printf("\n%s", *currPathPtr);
}

inline const char* PwdBrief(void)
{
    return "Print the current working directory.";
}