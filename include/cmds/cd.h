#pragma once
#include <uefi.h>
#include "shellutils.h"
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

    dirToChangeTo = MakeFullPath(args, *currPathPtr, &dynMemFlag);
    if (dirToChangeTo == NULL)
    {
        printf("\ncd: no directory specified");
        return;
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