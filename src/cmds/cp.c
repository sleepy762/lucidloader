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

    // If there are multiple sources then the destination is going to be a directory
    boolean_t dstIsDir = srcBegin->next == dstArg ? FALSE : TRUE;

    printf("%s\n", srcBegin->argString);

    return TRUE;
}

const char_t* CpBrief(void)
{
    return "Copy files.";
}

const char_t* CpLong(void)
{
    return "Usage: cp [-r] <src1> [src2]... <dst>";
}
