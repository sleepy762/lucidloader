#include "cmds/cat.h"

boolean_t CatCmd(cmd_args_s** args, char_t** currPathPtr)
{
    cmd_args_s* cmdArg = *args;
    cmd_args_s* arg = cmdArg->next;

    if (arg == NULL)
    {
        PrintCommandError(cmdArg->argString, NULL, CMD_NO_FILE_SPECIFIED);
        return FALSE;
    }

    boolean_t cmdSuccess = TRUE;

    // Print the content of each file in the arguments
    while(arg != NULL)
    {
        boolean_t isDynamicMemory = FALSE;

        char_t* filePath = MakeFullPath(arg->argString, *currPathPtr, &isDynamicMemory);
        if (filePath == NULL)
        {
            PrintCommandError(cmdArg->argString, NULL, CMD_NO_FILE_SPECIFIED);
            return FALSE;
        }

        int32_t res = PrintFileContent(filePath);
        if (res != CMD_SUCCESS)
        {
            PrintCommandError(cmdArg->argString, arg->argString, res);
            cmdSuccess = FALSE;
        }
        
        if (isDynamicMemory)
        {
            BS->FreePool(filePath);
        }
        arg = arg->next;
    }
    return cmdSuccess;
}

int32_t PrintFileContent(char_t* path)
{
    char_t* buffer = GetFileContent(path);
    if (buffer == NULL)
    {
        return errno;
    }

    printf("%s\n", buffer);
    BS->FreePool(buffer);
    
    return CMD_SUCCESS;
}

const char_t* CatBrief(void)
{
    return "Print the contents of a file.";
}

const char_t* CatLong(void)
{
    return "Usage: cat <file1> [file2] [file3] ...";
}
