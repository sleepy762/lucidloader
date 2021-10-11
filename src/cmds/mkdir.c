#include "cmds/mkdir.h"

int MkdirCmd(char args[], char** currPathPtr)
{
    if (args == NULL)
    {
        return CMD_NO_DIR_SPEFICIED;
    }

    boolean_t isDynamicMemory = FALSE;
    char* path = MakeFullPath(args, *currPathPtr, &isDynamicMemory);
    if (path == NULL)
    {
        return CMD_NO_DIR_SPEFICIED;
    }

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
            if (errno == EROFS)
            {
                return CMD_READ_ONLY_FILESYSTEM;
            }
            else
            {
                return CMD_GENERAL_DIR_OPENING_ERROR;
            }
        }
    }
    if (isDynamicMemory) BS->FreePool(path);

    return CMD_SUCCESS;
}

const char* MkdirBrief(void)
{
    return "Create a directory.";
}

const char* MkdirLong(void)
{
    return "Usage: mkdir <path or dirname>";
}
