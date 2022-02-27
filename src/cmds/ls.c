#include "cmds/ls.h"

uint8_t LsCmd(cmd_args_s** args, char_t** currPathPtr)
{
    cmd_args_s* cmdArg = *args;
    cmd_args_s* arg = cmdArg->next;
    if (arg == NULL) // If no arguments were passed
    {
        int32_t ret = ListDir(*currPathPtr);
        if (ret != CMD_SUCCESS)
        {
            PrintCommandError(cmdArg->argString, NULL, ret);
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }

    boolean_t cmdSuccess = TRUE;
    // Print each directory in the arguments
    while (arg != NULL)
    {
        boolean_t isDynamicMemory = FALSE;

        char_t* dirToList = MakeFullPath(arg->argString, *currPathPtr, &isDynamicMemory);
        if (dirToList != NULL)
        {
            uint8_t normalizationResult = NormalizePath(&dirToList);
            if (normalizationResult != CMD_SUCCESS)
            {
                PrintCommandError(cmdArg->argString, arg->argString, normalizationResult);
                return FALSE;
            }
        }
        else
        {
            PrintCommandError(cmdArg->argString, NULL, CMD_NO_DIR_SPEFICIED);
            return FALSE;
        }

        int32_t res = ListDir(dirToList);
        if (res != CMD_SUCCESS)
        {
            PrintCommandError(cmdArg->argString, arg->argString, res);
            cmdSuccess = FALSE;
        }
        else
        {
            printf("\n");
        }

        if (isDynamicMemory)
        {
            BS->FreePool(dirToList);
        }
        arg = arg->next;
    }
    return cmdSuccess;
}

int32_t ListDir(char_t* path)
{
    DIR* dir = opendir(path);
    if (dir != NULL)
    {
        struct dirent* de;

        printf("Reading the directory: %s\n", path);
        while ((de = readdir(dir)) != NULL)
        {
            printf("%c %04x %s\n", de->d_type == DT_DIR ? 'd' : '.', de->d_type, de->d_name);
        }
        closedir(dir);
    }
    else
    {
        return errno;
    }
    return CMD_SUCCESS;
}

const char_t* LsBrief(void)
{
    return "List files and directories.";
}

const char_t* LsLong(void)
{
    return "Usage: ls [path1] [path2] [path3] ...\n\
[pathN] - Optional argument where you can specify a path and multiple paths can be given.\n\
By default the current directory will be listed if no `path` argument is passed.";
}
