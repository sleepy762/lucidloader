#include "cmds/cp.h"

static int32_t CopyFileIntoDir(char_t* srcFile, char_t* dstDir);
static boolean_t CopyRecursively(char_t* mainPath, char_t* dstPath, cmd_args_s* cmdArg);

boolean_t CpCmd(cmd_args_s** args, char_t** currPathPtr)
{
    cmd_args_s* cmdArg = *args;

    boolean_t recursiveFlag = FindFlagAndDelete(args, RECURSIVE_FLAG);
    cmd_args_s* srcBegin = cmdArg->next;

    if (srcBegin == NULL)
    {
        PrintCommandError(cmdArg->argString, NULL, CMD_MISSING_SRC_FILE_OPERAND);
        return FALSE;
    }
    if (srcBegin->next == NULL)
    {
        PrintCommandError(cmdArg->argString, srcBegin->argString, CMD_MISSING_DST_FILE_OPERAND);
        return FALSE;
    }

    // The last argument is always the destination
    cmd_args_s* dstArg = GetLastArg(srcBegin);

    boolean_t isDynamicMemoryDstPath = FALSE;
    char_t* dstPath = MakeFullPath(dstArg->argString, *currPathPtr, &isDynamicMemoryDstPath);
    if (dstPath == NULL)
    {
        PrintCommandError(cmdArg->argString, dstArg->argString, CMD_MISSING_DST_FILE_OPERAND);
        return FALSE;
    }

    boolean_t cmdSuccess = TRUE;
    if (recursiveFlag) // Recursive copy
    {
        cmd_args_s* currSrc = srcBegin;
        
        while (currSrc != NULL)
        {
            // Reached the end of the sources
            if (currSrc == dstArg)
            {
                break;
            }

            boolean_t isDynamicMemory = FALSE;
            char_t* fullPath = MakeFullPath(currSrc->argString, *currPathPtr, &isDynamicMemory);
            if (fullPath == NULL)
            {
                PrintCommandError(cmdArg->argString, currSrc->argString, CMD_MISSING_SRC_FILE_OPERAND);
                return FALSE;
            }

            if (CopyRecursively(fullPath, dstPath, cmdArg) == FALSE)
            {
                cmdSuccess = FALSE;
            }

            if (isDynamicMemory)
            {
                free(fullPath);
            }
            currSrc = currSrc->next;
        }
    }
    else
    {
        // Simple copy of the kind `cp <src> <dst>`
        if (srcBegin->next == dstArg)
        {
            boolean_t isDynamicMemorySrcPath = FALSE;
            char_t* srcPath = MakeFullPath(srcBegin->argString, *currPathPtr, &isDynamicMemorySrcPath);
            if (srcPath == NULL)
            {
                PrintCommandError(cmdArg->argString, srcBegin->argString, CMD_MISSING_SRC_FILE_OPERAND);
                return FALSE;
            }

            char_t* fullDstPath = dstPath;

            FILE* fp = fopen(dstPath, "r");
            // If the destination is a directory, copy the file into the directory
            if (fp == NULL && errno == EISDIR)
            {
                fullDstPath = ConcatPaths(dstPath, strrchr(srcPath, '\\') + 1);
            }
            else if (fp != NULL)
            {
                fclose(fp);
            }

            int32_t res = CopyFile(srcPath, fullDstPath);
            if (res != 0)
            {
                PrintCommandError(cmdArg->argString, srcBegin->argString, res);
                cmdSuccess = FALSE;
            }

            if (fullDstPath != dstPath)
            {
                free(fullDstPath);
            }
            if (isDynamicMemorySrcPath)
            {
                free(srcPath);
            }
        }
        else // If there are multiple sources
        {
            // The destination has to be a directory
            int32_t res = CreateDirectory(dstPath);
            if (res != CMD_DIR_ALREADY_EXISTS && res != CMD_SUCCESS)
            {
                // Stop the command if the directory failed to be created and doesn't already exist
                PrintCommandError(cmdArg->argString, dstArg->argString, res);
                return FALSE;
            }

            // Iterate over all the sources
            cmd_args_s* currSrc = srcBegin;
            while (currSrc != NULL)
            {
                // Reached the end of the sources
                if (currSrc == dstArg)
                {
                    break;
                }

                boolean_t isDynamicMemorySrcPath = FALSE;
                char_t* srcPath = MakeFullPath(currSrc->argString, *currPathPtr, &isDynamicMemorySrcPath);
                if (srcPath == NULL)
                {
                    PrintCommandError(cmdArg->argString, currSrc->argString, CMD_MISSING_SRC_FILE_OPERAND);
                    return FALSE;
                }

                int32_t res = CopyFileIntoDir(srcPath, dstPath);
                if (res != 0)
                {
                    PrintCommandError(cmdArg->argString, currSrc->argString, res);
                    cmdSuccess = FALSE;
                }

                if (isDynamicMemorySrcPath)
                {
                    free(srcPath);
                }
                currSrc = currSrc->next;
            }
        }
    }
    if (isDynamicMemoryDstPath)
    {
        free(dstPath);
    }
    return cmdSuccess;
}

static int32_t CopyFileIntoDir(char_t* srcFile, char_t* dstDir)
{
    char_t* fileName = strrchr(srcFile, '\\') + 1;
    char_t* fullCopyPath = ConcatPaths(dstDir, fileName);

    int32_t res = CopyFile(srcFile, fullCopyPath);
    free(fullCopyPath);
    return res;
}

static boolean_t CopyRecursively(char_t* mainPath, char_t* dstPath, cmd_args_s* cmdArg)
{
    DIR* dir = opendir(mainPath);
    // Allow copying normal files with the recursive flag on
    if (dir == NULL && errno == ENOTDIR)
    {
        int32_t res = CreateDirectory(dstPath);
        if (res != CMD_DIR_ALREADY_EXISTS && res != CMD_SUCCESS)
        {
            PrintCommandError(cmdArg->argString, dstPath, res);
            return FALSE;
        }

        res = CopyFileIntoDir(mainPath, dstPath);
        if (res != 0)
        {
            PrintCommandError(cmdArg->argString, mainPath, res);
            return FALSE;
        }
        return TRUE;
    }
    else if (dir == NULL)
    {
        PrintCommandError(cmdArg->argString, mainPath, errno);
        return FALSE;
    }

    // Make sure the base destination directory exists
    int32_t dstPathRes = CreateDirectory(dstPath);
    if (dstPathRes != CMD_DIR_ALREADY_EXISTS && dstPathRes != CMD_SUCCESS)
    {
        PrintCommandError(cmdArg->argString, dstPath, dstPathRes);
        return FALSE;
    }

    // Use the new destination
    char_t* newDstPath = ConcatPaths(dstPath, strrchr(mainPath, '\\') + 1);
    dstPathRes = CreateDirectory(newDstPath);
    if (dstPathRes != CMD_DIR_ALREADY_EXISTS && dstPathRes != CMD_SUCCESS)
    {
        PrintCommandError(cmdArg->argString, newDstPath, dstPathRes);
        return FALSE;
    }

    boolean_t funcSuccess = TRUE;
    struct dirent* de;
    while ((de = readdir(dir)) != NULL)
    {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
        {
            // Skip the current and previous directories
            continue;
        }

        boolean_t isDynamicMemory = FALSE;
        // Concatenate the current directory path with the name of the file to copy
        char_t* filePath = MakeFullPath(de->d_name, mainPath, &isDynamicMemory);

        // If we find a directory, descend into it and copy everything in it
        if (de->d_type == DT_DIR)
        {
            if (CopyRecursively(filePath, newDstPath, cmdArg) == FALSE)
            {
                funcSuccess = FALSE;
            }
        }
        else
        {
            int32_t res = CopyFileIntoDir(filePath, newDstPath);
            if (res != 0)
            {
                PrintCommandError(cmdArg->argString, filePath, res);
                funcSuccess = FALSE;
            }
        }

        if (isDynamicMemory)
        {
            free(filePath);
        }
    }
    free(newDstPath);
    return funcSuccess;
}

const char_t* CpBrief(void)
{
    return "Copy files.";
}

const char_t* CpLong(void)
{
    return "Usage: cp [-r] <src1> [src2]... <dst>";
}
