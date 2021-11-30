#include "cmds/rm.h"

uint8_t RmCmd(cmd_args_s** args, char_t** currPathPtr)
{
    if (*args == NULL)
    {
        return CMD_NO_FILE_SPECIFIED;
    }

    boolean_t recursiveFlag = FindFlagAndDelete(args, "-r");

    cmd_args_s* arg = *args;
    while (arg != NULL)
    {
        boolean_t isDynamicMemory = FALSE;

        char_t* filePath = MakeFullPath(arg->argString, *currPathPtr, &isDynamicMemory);
        if (filePath == NULL)
        {
            return CMD_NO_FILE_SPECIFIED;
        }

        int32_t ret = RemoveFile(filePath);
        if (ret != 0)
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

const char_t* RmBrief(void)
{
    return "Delete files or directories.";
}

const char_t* RmLong(void)
{
    return "Usages: rm <file1> [file2] [file3] ...\n\
OR: rm -r <directory1> [directory2] [directory3] ...";
}
