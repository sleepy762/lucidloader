#include "cmds/cat.h"

uint8_t CatCmd(cmd_args_s** args, char_t** currPathPtr)
{
    if (*args == NULL)
    {
        return CMD_NO_FILE_SPECIFIED;
    }

    // Print the content of each file in the arguments
    cmd_args_s* arg = *args;
    while(arg != NULL)
    {
        boolean_t isDynamicMemory = FALSE;

        char_t* filePath = MakeFullPath(arg->argString, *currPathPtr, &isDynamicMemory);
        if (filePath == NULL)
        {
            return CMD_NO_FILE_SPECIFIED;
        }

        uint8_t res = PrintFileContent(filePath);
        if (res != CMD_SUCCESS)
        {
            PrintCommandError("cat", arg->argString, res);
        }
        
        if (isDynamicMemory)
        {
            BS->FreePool(filePath);
        }
        arg = arg->next;
    }
    return CMD_SUCCESS;
}

uint8_t PrintFileContent(char_t* path)
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
