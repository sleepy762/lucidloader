#include "cmds/ls.h"

uint8_t LsCmd(char_t args[], char_t** currPathPtr)
{
    char_t* dirToList = NULL;
    boolean_t isDynamicMemory = FALSE;
    
    if (args != NULL)
    {
        dirToList = MakeFullPath(args, *currPathPtr, &isDynamicMemory);
        if (dirToList != NULL)
        {
            uint8_t normalizationResult = NormalizePath(&dirToList);
            if (normalizationResult != CMD_SUCCESS)
            {
                return normalizationResult;
            }
        }
        else
        {
            dirToList = *currPathPtr;
        }
    }
    else // if the user didn't pass arguments
    {
        dirToList = *currPathPtr;
    }

    DIR* dir = opendir(dirToList);
    if (dir != NULL)
    {
        struct dirent* de;

        printf("\nReading the directory: %s\n", dirToList);
        while ((de = readdir(dir)) != NULL)
        {
            printf("%c %04x %s\n", de->d_type == DT_DIR ? 'd' : '.', de->d_type, de->d_name);
        }
        closedir(dir);
    }
    else
    {
        return CMD_CANT_READ_DIR;
    }
    if (isDynamicMemory) BS->FreePool(dirToList);

    return CMD_SUCCESS;
}

const char_t* LsBrief(void)
{
    return "List files and directories.";
}

const char_t* LsLong(void)
{
    return "Usage: ls [path]\n\
[path] - Optional argument where you can specify a path.\n\
By default the current directory will be listed if no `path` argument is passed.";
}
