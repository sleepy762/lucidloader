#include "cmds/mkdir.h"

boolean_t MkdirCmd(cmd_args_s** args, char_t** currPathPtr)
{
    cmd_args_s* cmdArg = *args;
    cmd_args_s* arg = cmdArg->next;
    if (arg == NULL)
    {
        PrintCommandError(cmdArg->argString, NULL, CMD_NO_DIR_SPEFICIED);
        return FALSE;
    }

    boolean_t cmdSuccess = TRUE;
    // Create each directory in the arguments
    while (arg != NULL)
    {
        boolean_t isDynamicMemory = FALSE;

        char_t* path = MakeFullPath(arg->argString, *currPathPtr, &isDynamicMemory);
        if (path == NULL)
        {
            PrintCommandError(cmdArg->argString, NULL, CMD_NO_DIR_SPEFICIED);
            return FALSE;
        }

        int32_t res = CreateDirectory(path);
        if (res != CMD_SUCCESS)
        {
            PrintCommandError(cmdArg->argString, arg->argString, res);
            cmdSuccess = FALSE;
        }
        
        if (isDynamicMemory) 
        {
            BS->FreePool(path);
        }
        arg = arg->next;
    }
    return cmdSuccess;
}

const char_t* MkdirBrief(void)
{
    return "Create a directory.";
}

const char_t* MkdirLong(void)
{
    return "Usage: mkdir <path1> [path2] [path3] ...";
}
