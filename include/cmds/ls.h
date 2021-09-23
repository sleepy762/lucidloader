#pragma once
#include <uefi.h>
#include "bootutils.h"

inline void LsCmd(char args[], char** currPathPtr)
{
    DIR* dir;
    struct dirent* de;

    DIR* auxDir;
    char* dirToList = NULL;
    boolean_t dynMemFlag = FALSE;
    if (args)
    {
        size_t argsLen = strlen(args);
        char* originalArgs = args;

        // remove leading whitespace
        while(isspace(*args)) args++;

        // remove trailing whitespace
        char* end = originalArgs + argsLen - 1;
        while(end > originalArgs && isspace(*end)) end--;
        end[1] = 0;

        // Remove duplicate backslashes from the command
        RemoveRepeatedChars(args, DIRECTORY_DELIM);

        // Check if the path starts from the root dir
        if (args[0] == DIRECTORY_DELIM)
        {
            dirToList = args;
        }
        // Check the concatenated path
        else
        {
            dirToList = ConcatPaths(*currPathPtr, args);
            dynMemFlag = TRUE;
        }
        NormalizePath(&dirToList);
    }
    else // if the user didn't pass arguments
    {
        dirToList = *currPathPtr;
    }

    if ((dir = opendir(dirToList)))
    {
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

    if (dynMemFlag) BS->FreePool(dirToList);
}

inline const char* LsBrief(void)
{
    return "List files and directories.";
}

inline const char* LsLong(void)
{
    return "Usage: ls [path]\n\
[path] - Optional argument where you can specify a path.\n\
By default the current directory will be listed if no `path` argument is passed.";
}