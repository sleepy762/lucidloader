#include "cmds/touch.h"

int TouchCmd(char args[], char** currPathPtr)
{
    if (args == NULL)
    {
        return CMD_NO_FILE_SPECIFIED;
    }

    boolean_t isDynamicMemory = FALSE;

    char* path = MakeFullPath(args, *currPathPtr, &isDynamicMemory);
    if (path == NULL)
    {
        return CMD_NO_FILE_SPECIFIED;
    }

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
        if (errno == EROFS)
        {
            return CMD_READ_ONLY_FILESYSTEM;
        }
        else
        {
            return CMD_GENERAL_FILE_OPENING_ERROR;
        }
    }
    if (isDynamicMemory) BS->FreePool(path);
    
    return CMD_SUCCESS;
}

const char* TouchBrief(void)
{
    return "Create a file.";
}

const char* TouchLong(void)
{
    return "Usage: touch <path or filename>";
}
