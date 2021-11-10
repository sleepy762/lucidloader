#include "cmds/mkdir.h"

uint8_t MkdirCmd(cmd_args_s* args, char_t** currPathPtr)
{
    if (args == NULL)
    {
        return CMD_NO_DIR_SPEFICIED;
    }

    // Create each directory in the arguments
    while (args != NULL)
    {
        boolean_t isDynamicMemory = FALSE;

        char_t* path = MakeFullPath(args->argString, *currPathPtr, &isDynamicMemory);
        if (path == NULL)
        {
            return CMD_NO_DIR_SPEFICIED;
        }

        uint8_t res = ReadDirectory(path);
        if (res != CMD_SUCCESS)
        {
            PrintCommandError("mkdir", args->argString, res);
        }
        
        if (isDynamicMemory) 
        {
            BS->FreePool(path);
        }
        args = args->next;
    }

    return CMD_SUCCESS;
}

uint8_t ReadDirectory(char_t* path)
{
    DIR* dir = opendir(path);
    if (dir != NULL)
    {
        closedir(dir);
        return CMD_DIR_ALREADY_EXISTS;
    }
    else
    {
        // Creates a new directory and frees the pointer to it
        FILE* fp = fopen(path, "wd");
        if (fp != NULL)
        {
            fclose(fp);
        }
        else
        {
            // Make the error message more sensible
            return (errno == ENOTDIR) ? EEXIST : errno;
        }
    }
    return CMD_SUCCESS;
}

const char_t* MkdirBrief(void)
{
    return "Create a directory.";
}

const char_t* MkdirLong(void)
{
    return "Usage: mkdir <path1> [path2] [path3] ...";
}
