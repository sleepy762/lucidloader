#include "cmds/touch.h"

uint8_t TouchCmd(char_t args[], char_t** currPathPtr)
{
    if (args == NULL)
    {
        return CMD_NO_FILE_SPECIFIED;
    }

    boolean_t isDynamicMemory = FALSE;

    char_t* path = MakeFullPath(args, *currPathPtr, &isDynamicMemory);
    if (path == NULL)
    {
        return CMD_NO_FILE_SPECIFIED;
    }

    // Prevent overriding an existing file
    FILE* fp = fopen(path, "r");
    if (fp == NULL)
    {
        fp = fopen(path, "w");
    }

    if (fp != NULL)
    {
        fclose(fp);
    }
    else
    {
        return errno;
    }
    if (isDynamicMemory) BS->FreePool(path);
    
    return CMD_SUCCESS;
}

const char_t* TouchBrief(void)
{
    return "Create a file.";
}

const char_t* TouchLong(void)
{
    return "Usage: touch <path or filename>";
}
