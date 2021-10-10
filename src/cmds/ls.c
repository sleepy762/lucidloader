#include "cmds/ls.h"

void LsCmd(char args[], char** currPathPtr)
{
    char* dirToList = NULL;
    boolean_t isDynamicMemory = FALSE;
    if (args != NULL)
    {
        dirToList = MakeFullPath(args, *currPathPtr, &isDynamicMemory);
        if (dirToList != NULL)
        {
            NormalizePath(&dirToList);
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
        printf("\nls: unable to read directory");
    }

    if (isDynamicMemory) BS->FreePool(dirToList);
}

const char* LsBrief(void)
{
    return "List files and directories.";
}

const char* LsLong(void)
{
    return "Usage: ls [path]\n\
[path] - Optional argument where you can specify a path.\n\
By default the current directory will be listed if no `path` argument is passed.";
}
