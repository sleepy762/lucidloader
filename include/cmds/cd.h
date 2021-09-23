#pragma once
#include <uefi.h>
#include "bootutils.h"

inline void CdCmd(char args[], char** currPathPtr)
{
    if (!args)
    {
        printf("\ncd: no directory specified");
        return;
    }

    DIR* auxDir;
    char* dirToChangeTo = NULL;
    boolean_t dynMemFlag = FALSE;

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

    // Remove a backslash from the end if it exists
    if (end[0] == DIRECTORY_DELIM && strlen(args) > 1) end[0] = 0;

    // Check if the path starts from the root dir
    if (args[0] == DIRECTORY_DELIM)
    {
        dirToChangeTo = args;
    }
    // if the args are only whitespace
    else if(args[0] == 0)
    {
        printf("\ncd: no directory specified");
        return;
    }
    else // Check the concatenated path
    {
        dirToChangeTo = ConcatPaths(*currPathPtr, args);
        dynMemFlag = TRUE;
    }

    NormalizePath(&dirToChangeTo);

    // Try to open the directory to make sure it exists
    if ((auxDir = opendir(dirToChangeTo)))
    {
        closedir(auxDir);
        BS->FreePool(*currPathPtr);

        if (dynMemFlag)
        {
            *currPathPtr = dirToChangeTo;
        }
        else // if the string wasn't allocated dynamically, we have to do that
        {
            size_t newDirLen = strlen(dirToChangeTo);
            efi_status_t status = BS->AllocatePool(LIP->ImageDataType, newDirLen + 1, (void**)currPathPtr);
            if (EFI_ERROR(status))
                ErrorExit("cd: Failed to allocate memory.", status);
            memcpy(*currPathPtr, dirToChangeTo, newDirLen + 1);
        }
    }
    else
    {
        printf("\ncd: directory `%s` not found", dirToChangeTo);
    }
}

inline const char* CdBrief(void)
{
    return "Change the current working directory.";
}

inline const char* CdLong(void)
{
    return "Usage: cd <directory>";
}