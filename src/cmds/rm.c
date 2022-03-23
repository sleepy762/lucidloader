#include "cmds/rm.h"
#include "shellerr.h"
#include "shellutils.h"
#include "bootutils.h"

#define RECURSIVE_FLAG ("-r")

static boolean_t RemoveRecursively(char_t* mainPath, cmd_args_s* cmdArg);


boolean_t RmCmd(cmd_args_s** args, char_t** currPathPtr)
{
    cmd_args_s* cmdArg = *args;

    boolean_t recursiveFlag = FindFlagAndDelete(args, RECURSIVE_FLAG);
    cmd_args_s* arg = cmdArg->next;
    
    if (arg == NULL)
    {
        PrintCommandError(cmdArg->argString, NULL, CMD_NO_FILE_SPECIFIED);
        return FALSE;
    }

    boolean_t cmdSuccess = TRUE;
    while (arg != NULL)
    {
        // Refuse to remove '.' or '..' directories
        if (strcmp(arg->argString, ".") == 0 || strcmp(arg->argString, "..") == 0)
        {
            PrintCommandError(cmdArg->argString, arg->argString, CMD_REFUSE_REMOVE);
            arg = arg->next;
            cmdSuccess = FALSE;
            continue;
        }

        boolean_t isDynamicMemory = FALSE;
        char_t* filePath = MakeFullPath(arg->argString, *currPathPtr, &isDynamicMemory);
        if (filePath == NULL)
        {
            PrintCommandError(cmdArg->argString, NULL, CMD_NO_FILE_SPECIFIED);
            return FALSE;
        }

        if (recursiveFlag)
        {
            if (RemoveRecursively(filePath, cmdArg) == FALSE)
            {
                cmdSuccess = FALSE;
            }
        }
        else
        {
            if (remove(filePath) != 0)
            {
                PrintCommandError(cmdArg->argString, arg->argString, errno);
                cmdSuccess = FALSE;
            }
        }

        if (isDynamicMemory)
        {
            free(filePath);
        }
        arg = arg->next;
    }
    return cmdSuccess;
}

static boolean_t RemoveRecursively(char_t* mainPath, cmd_args_s* cmdArg)
{
    DIR* dir = opendir(mainPath);
    // Allow deleting normal files with the recursive flag on
    if (dir == NULL && errno == ENOTDIR)
    {
        return (remove(mainPath) == 0);
    }
    else if (dir == NULL)
    {
        return FALSE;
    }

    boolean_t funcSuccess = TRUE;
    struct dirent* de;
    // This loop looks similar to the loop in the main RmCmd function, but it's in fact different
    while ((de = readdir(dir)) != NULL)
    {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
        {
            // Skip the current and previous directories
            continue;
        }

        boolean_t isDynamicMemory = FALSE;
        // Concatenate the current directory path with the name of the file to delete
        char_t* filePath = MakeFullPath(de->d_name, mainPath, &isDynamicMemory);

        // If we find a directory, we descend into it and delete everything in it
        if (de->d_type == DT_DIR)
        {
            if (RemoveRecursively(filePath, cmdArg) == FALSE)
            {
                funcSuccess = FALSE;
            }
        }
        else
        {
            if (remove(filePath) != 0)
            {
                PrintCommandError(cmdArg->argString, filePath, errno);
            }
        }

        if (isDynamicMemory)
        {
            free(filePath);
        }
    }
    // Remove the parent directory
    if (remove(mainPath) != 0)
    {
        funcSuccess = FALSE;
    }
    return funcSuccess;
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
