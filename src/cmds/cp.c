#include "cmds/cp.h"

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

    boolean_t cmdSuccess = TRUE;
    if (recursiveFlag) // Recursive copy
    {
        // recursive copy function
        // dst is always dir, unless `cp -r <src file> <dst file>`
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

            boolean_t isDynamicMemoryDstPath = FALSE;
            char_t* dstPath = MakeFullPath(dstArg->argString, *currPathPtr, &isDynamicMemoryDstPath);
            if (dstPath == NULL)
            {
                PrintCommandError(cmdArg->argString, dstArg->argString, CMD_MISSING_DST_FILE_OPERAND);
                return FALSE;
            }

            int32_t res = CopyFile(srcPath, dstPath);
            if (res != 0)
            {
                PrintCommandError(cmdArg->argString, srcBegin->argString, res);
                cmdSuccess = FALSE;
            }

            if (isDynamicMemorySrcPath)
            {
                BS->FreePool(srcPath);
            }
            if (isDynamicMemoryDstPath)
            {
                BS->FreePool(dstPath);
            }
        }
        else // If there are multiple sources
        {
            boolean_t isDynamicMemoryDstPath = FALSE;
            char_t* dstPath = MakeFullPath(dstArg->argString, *currPathPtr, &isDynamicMemoryDstPath);
            if (dstPath == NULL)
            {
                PrintCommandError(cmdArg->argString, dstArg->argString, CMD_MISSING_DST_FILE_OPERAND);
                return FALSE;
            }

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

                char_t* fileName = strrchr(srcPath, '\\');
                char_t* fullDstPath = ConcatPaths(dstPath, fileName);

                int32_t res = CopyFile(srcPath, fullDstPath);
                if (res != 0)
                {
                    PrintCommandError(cmdArg->argString, currSrc->argString, res);
                    cmdSuccess = FALSE;
                }

                if (isDynamicMemorySrcPath)
                {
                    BS->FreePool(srcPath);
                }
                BS->FreePool(fullDstPath);
                currSrc = currSrc->next;
            }

            if (isDynamicMemoryDstPath)
            {
                BS->FreePool(dstPath);
            }
        }
    }
    return cmdSuccess;
}

const char_t* CpBrief(void)
{
    return "Copy files.";
}

const char_t* CpLong(void)
{
    return "Usage: cp [-r] <src1> [src2]... <dst>";
}
