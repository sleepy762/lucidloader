#include "cmds/cd.h"

void CdCmd(char args[], char** currPathPtr)
{
    if (!args)
    {
        printf("\ncd: no directory specified");
        return;
    }

    DIR* auxDir;
    boolean_t isDynamicMemory = FALSE;

    char* dirToChangeTo = MakeFullPath(args, *currPathPtr, &isDynamicMemory);
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

        if (isDynamicMemory)
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

const char* CdBrief(void)
{
    return "Change the current working directory.";
}

const char* CdLong(void)
{
    return "Usage: cd <directory>";
}
