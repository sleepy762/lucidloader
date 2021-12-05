#include "cmds/rm.h"

uint8_t RmCmd(cmd_args_s** args, char_t** currPathPtr)
{
    if (*args == NULL)
    {
        return CMD_NO_FILE_SPECIFIED;
    }

    boolean_t recursiveFlag = FindFlagAndDelete(args, RECURSIVE_FLAG);

    cmd_args_s* arg = *args;
    while (arg != NULL)
    {
        // Refuse to remove '.' or '..' directories
        if (strcmp(arg->argString, ".") == 0 || strcmp(arg->argString, "..") == 0)
        {
            PrintCommandError("rm", arg->argString, CMD_REFUSE_REMOVE);
            arg = arg->next;
            continue;
        }

        boolean_t isDynamicMemory = FALSE;

        char_t* filePath = MakeFullPath(arg->argString, *currPathPtr, &isDynamicMemory);
        if (filePath == NULL)
        {
            return CMD_NO_FILE_SPECIFIED;
        }

        int32_t res;
        if (recursiveFlag)
        {
            res = RemoveDirRecursively(filePath);
        }
        else
        {
            res = remove(filePath);
        }

        if (res != 0)
        {
            PrintCommandError("rm", arg->argString, errno);
        }

        if (isDynamicMemory)
        {
            BS->FreePool(filePath);
        }
        arg = arg->next;
    }

    return CMD_SUCCESS;
}

uint8_t RemoveDirRecursively(char_t* mainPath)
{
    DIR* dir = opendir(mainPath);
    // Allow deleting normal files with the recursive flag on
    if (dir == NULL && errno == ENOTDIR)
    {
        return (remove(mainPath) != 0) ? 1 : 0;
    }
    else if (dir == NULL)
    {
        return 1;
    }

    struct dirent* de;
    // This loop looks similar to the loop in the main RmCmd function, but it's in fact different
    while ((de = readdir(dir)) != NULL)
    {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
        {
            // Skip the current and previous directory
            continue;
        }
        int32_t res;

        boolean_t isDynamicMemory = FALSE;
        // Concatenate the current directory path with the name of the directory to delete
        char_t* filePath = MakeFullPath(de->d_name, mainPath, &isDynamicMemory);

        // If we find a directory, we descend into it and delete everything in it
        if (de->d_type == DT_DIR)
        {
            res = RemoveDirRecursively(filePath);
        }
        else
        {
            res = remove(filePath);
        }

        if (res != 0)
        {
            PrintCommandError("rm", filePath, errno);
        }

        if (isDynamicMemory)
        {
            BS->FreePool(filePath);
        }
    }
    // Remove the parent directory
    return (remove(mainPath) != 0) ? 1 : 0;
}

const char_t* RmBrief(void)
{
    return "Delete files or directories.";
}

const char_t* RmLong(void)
{
    return "Usages: rm <file1> [file2] [file3] ...\n\
OR: rm -r <directory1> [directory2] [directory3] ...";
}
