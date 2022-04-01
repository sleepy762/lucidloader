#include "cmds/cd.h"
#include "shellutils.h"
#include "bootutils.h"
#include "shellerr.h"

boolean_t CdCmd(cmd_args_s** args, char_t** currPathPtr)
{
    cmd_args_s* cmdArg = *args;
    cmd_args_s* arg = cmdArg->next;

    if (arg == NULL)
    {
        PrintCommandError(cmdArg->argString, NULL, CMD_NO_DIR_SPEFICIED);
        return FALSE;
    }

    boolean_t isDynamicMemory = FALSE;

    char_t* dirToChangeTo = MakeFullPath(arg->argString, *currPathPtr, &isDynamicMemory);
    if (dirToChangeTo == NULL)
    {
        PrintCommandError(cmdArg->argString, arg->argString, CMD_NO_DIR_SPEFICIED);
        return FALSE;
    }

    uint8_t normalizationResult = NormalizePath(&dirToChangeTo);
    if (normalizationResult != CMD_SUCCESS)
    {
        PrintCommandError(cmdArg->argString, arg->argString, normalizationResult);
        return FALSE;    
    }

    // Try to open the directory to make sure it exists
    DIR* auxDir = opendir(dirToChangeTo);
    if (auxDir != NULL)
    {
        closedir(auxDir);
        free(*currPathPtr);

        if (isDynamicMemory)
        {
            *currPathPtr = dirToChangeTo;
        }
        else // if the string wasn't allocated dynamically, we have to do that
        {
            size_t newDirLen = strlen(dirToChangeTo);
            *currPathPtr = malloc(newDirLen + 1);
            if (*currPathPtr == NULL)
            {
                PrintCommandError(cmdArg->argString, arg->argString, CMD_OUT_OF_MEMORY);
                return FALSE;
            }
            memcpy(*currPathPtr, dirToChangeTo, newDirLen + 1);
        }
    }
    else
    {
        PrintCommandError(cmdArg->argString, arg->argString, errno);
        return FALSE;
    }
    return TRUE;
}

const char_t* CdBrief(void)
{
    return "Change the current working directory.";
}

const char_t* CdLong(void)
{
    return "Usage: cd <directory>";
}
