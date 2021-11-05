#include "cmds/mkdir.h"

uint8_t MkdirCmd(char_t args[], char_t** currPathPtr)
{
    if (args == NULL)
    {
        return CMD_NO_DIR_SPEFICIED;
    }

    boolean_t isDynamicMemory = FALSE;
    char_t* path = MakeFullPath(args, *currPathPtr, &isDynamicMemory);
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
            // Make the error message more sensible
            if (errno == ENOTDIR)
            {
                return EEXIST;
            }
            return errno;
        }
    }
    if (isDynamicMemory) BS->FreePool(path);

    return CMD_SUCCESS;
}

const char_t* MkdirBrief(void)
{
    return "Create a directory.";
}

const char_t* MkdirLong(void)
{
    return "Usage: mkdir <path or dirname>";
}
