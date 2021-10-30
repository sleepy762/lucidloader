#include "cmds/cd.h"

uint8_t CdCmd(char_t args[], char_t** currPathPtr)
{
    if (args == NULL)
    {
        return CMD_NO_DIR_SPEFICIED;
    }

    boolean_t isDynamicMemory = FALSE;

    char_t* dirToChangeTo = MakeFullPath(args, *currPathPtr, &isDynamicMemory);
    if (dirToChangeTo == NULL)
    {
        return CMD_NO_DIR_SPEFICIED;
    }

    uint8_t normalizationResult = NormalizePath(&dirToChangeTo);
    if (normalizationResult != CMD_SUCCESS)
    {
        return normalizationResult;
    }

    // Try to open the directory to make sure it exists
    DIR* auxDir = opendir(dirToChangeTo);
    if (auxDir != NULL)
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
            {
                return CMD_OUT_OF_MEMORY;
            }
            memcpy(*currPathPtr, dirToChangeTo, newDirLen + 1);
        }
    }
    else
    {
        return CMD_DIR_NOT_FOUND;
    }
    return CMD_SUCCESS;
}

const char_t* CdBrief(void)
{
    return "Change the current working directory.";
}

const char_t* CdLong(void)
{
    return "Usage: cd <directory>";
}
