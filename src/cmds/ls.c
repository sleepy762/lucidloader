#include "cmds/ls.h"

uint8_t LsCmd(cmd_args_s** args, char_t** currPathPtr)
{
    if (*args == NULL) // If no arguments were passed
    {
        return ListDir(*currPathPtr);
    }
    else
    {
        // Print each directory in the arguments
        cmd_args_s* arg = *args;
        while (arg != NULL)
        {
            boolean_t isDynamicMemory = FALSE;

            char_t* dirToList = MakeFullPath(arg->argString, *currPathPtr, &isDynamicMemory);
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
                return CMD_OUT_OF_MEMORY;
            }

            uint8_t res = ListDir(dirToList);
            if (res != CMD_SUCCESS)
            {
                PrintCommandError("ls", arg->argString, res);
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
    }

    return CMD_SUCCESS;
}

uint8_t ListDir(char_t* path)
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
