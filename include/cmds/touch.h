#pragma once
#include <uefi.h>
#include "bootutils.h"

inline void TouchCmd(char args[], char** currPathPtr)
{
    if(!args)
    {
        printf("\ntouch: no filename specified");
        return;
    }

    CleanPath(&args);
    char* path = NULL;
    boolean_t dynMemFlag = FALSE;

    if (args[0] == DIRECTORY_DELIM)
    {
        path = args;
    }
    else if (args[0] == 0)
    {
        printf("\ntouch: no filename specified");
        return;
    }
    else
    {
        path = ConcatPaths(*currPathPtr, args);
        dynMemFlag = TRUE;
    }

    FILE* fp = fopen(path, "r");
    if(fp == NULL)
    {
        fp = fopen(path, "w");
    }

    if (fp)
    {
        fclose(fp);
    }
    else
    {
        if(errno == EROFS)
            printf("\ntouch: permission denied - readonly filesystem");
        else
            printf("\ntouch: failed to open file");
    }
    if (dynMemFlag) BS->FreePool(path);
}

inline const char* TouchBrief(void)
{
    return "Create a file if it doesn't exist.";
}

inline const char* TouchLong(void)
{
    return "Usage: touch <filename>";
}