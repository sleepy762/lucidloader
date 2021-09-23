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

    // Check if the path starts from the root dir
    if (args[0] == '\\')
    {
        dirToChangeTo = args;
    }
    // Check the concatenated path
    else
    {
        dirToChangeTo = ConcatPaths(*currPathPtr, args);
        dynMemFlag = TRUE;
    }

    if ((auxDir = opendir(dirToChangeTo)))
    {
        closedir(auxDir);
        BS->FreePool(*currPathPtr);

        if (dynMemFlag)
        {
            *currPathPtr = dirToChangeTo;
        }
        else
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