#include "cmds/edit.h"

boolean_t EditCmd(cmd_args_s** args, char_t** currPathPtr)
{
    cmd_args_s* cmdArg = *args;
    cmd_args_s* arg = cmdArg->next;

    boolean_t isDynamicMemory = FALSE;
    char_t* filePath = NULL;

    if (arg != NULL)
    {
        filePath = MakeFullPath(arg->argString, *currPathPtr, &isDynamicMemory);
        if (filePath == NULL)
        {
            PrintCommandError(cmdArg->argString, NULL, CMD_NO_FILE_SPECIFIED);
            return FALSE;
        }
    }

    int8_t res = StartEditor(filePath);
    if (res == -1)
    {
        PrintCommandError(cmdArg->argString, arg->argString, CMD_EFI_FAIL);
        return FALSE;
    }

    if (isDynamicMemory)
    {
        BS->FreePool(filePath);
    }

    return TRUE;
}

const char_t* EditBrief(void)
{
    return "Text editor.";
}

const char_t* EditLong(void)
{
    return "Usage: edit [filename]";
}
