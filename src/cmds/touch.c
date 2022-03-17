#include "cmds/touch.h"

static int32_t CreateFileNoOverride(char_t* path);

boolean_t TouchCmd(cmd_args_s** args, char_t** currPathPtr)
{
    cmd_args_s* cmdArg = *args;
    cmd_args_s* arg = cmdArg->next;
    if (arg == NULL)
    {
        PrintCommandError(cmdArg->argString, NULL, CMD_NO_FILE_SPECIFIED);
        return FALSE;
    }

    boolean_t cmdSuccess = TRUE;
    // Create each file in the arguments
    while (arg != NULL)
    {
        boolean_t isDynamicMemory = FALSE;

        char_t* path = MakeFullPath(arg->argString, *currPathPtr, &isDynamicMemory);
        if (path == NULL)
        {
            PrintCommandError(cmdArg->argString, NULL, CMD_NO_FILE_SPECIFIED);
            return FALSE;
        }

        int32_t res = CreateFileNoOverride(path);
        if (res != CMD_SUCCESS)
        {
            PrintCommandError(cmdArg->argString, arg->argString, res);
            cmdSuccess = FALSE;
        }

        if (isDynamicMemory) 
        {
            free(path);
        }
        arg = arg->next;
    }
    return cmdSuccess;
}

static int32_t CreateFileNoOverride(char_t* path)
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
