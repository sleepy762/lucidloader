#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "bootutils.h"

inline void TouchCmd(char args[], char** currPathPtr)
{
    if(!args)
    {
        printf("\ntouch: no filename specified");
        return;
    }

    char* path = NULL;
    boolean_t dynMemFlag = FALSE;

    path = MakeFullPath(args, *currPathPtr, &dynMemFlag);
    if (path == NULL)
    {
        printf("\ntouch: no filename specified");
        return;
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