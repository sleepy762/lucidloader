#include "cmds/touch.h"

uint8_t TouchCmd(cmd_args_s* args, char_t** currPathPtr)
{
    if (args == NULL)
    {
        return CMD_NO_FILE_SPECIFIED;
    }

    // Create each file in the arguments
    while (args != NULL)
    {
        boolean_t isDynamicMemory = FALSE;

        char_t* path = MakeFullPath(args->argString, *currPathPtr, &isDynamicMemory);
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

        if (isDynamicMemory) 
        {
            BS->FreePool(path);
        }
        args = args->next;
    }
    
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
