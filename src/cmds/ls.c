#include "cmds/ls.h"

uint8_t LsCmd(cmd_args_s* args, char_t** currPathPtr)
{
    if (args == NULL) // If no arguments were passed
    {
        return ListDir(*currPathPtr);
    }
    else
    {
        // Print each directory in the arguments
        while (args != NULL)
        {
            boolean_t isDynamicMemory = FALSE;

            char_t* dirToList = MakeFullPath(args->argString, *currPathPtr, &isDynamicMemory);
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
                PrintCommandError("ls", args->argString, res);
            }

            if (isDynamicMemory) 
            {
                BS->FreePool(dirToList);
            }
            args = args->next;
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

        printf("\nReading the directory: %s\n", path);
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
    return "Usage: ls [path]\n\
[path] - Optional argument where you can specify a path.\n\
By default the current directory will be listed if no `path` argument is passed.";
}
