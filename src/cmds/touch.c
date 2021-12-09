#include "cmds/touch.h"

uint8_t TouchCmd(cmd_args_s** args, char_t** currPathPtr)
{
    if (*args == NULL)
    {
        return CMD_NO_FILE_SPECIFIED;
    }

    // Create each file in the arguments
    cmd_args_s* arg = *args;
    while (arg != NULL)
    {
        boolean_t isDynamicMemory = FALSE;

        char_t* path = MakeFullPath(arg->argString, *currPathPtr, &isDynamicMemory);
        if (path == NULL)
        {
            return CMD_NO_FILE_SPECIFIED;
        }

        uint8_t res = CreateFile(path);
        if (res != CMD_SUCCESS)
        {
            PrintCommandError("touch", arg->argString, res);
        }

        if (isDynamicMemory) 
        {
            BS->FreePool(path);
        }
        arg = arg->next;
    }
    
    return CMD_SUCCESS;
}

uint8_t CreateFile(char_t* path)
{
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
    return CMD_SUCCESS;
}

const char_t* TouchBrief(void)
{
    return "Create a file.";
}

const char_t* TouchLong(void)
{
    return "Usage: touch <path1> [path2] [path3] ...";
}
