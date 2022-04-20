#include "cmds/edit.h"
#include "editor.h"
#include "bootutils.h"
#include "shellerr.h"
#include "shellutils.h"

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
    if (res != 0)
    {
        PrintCommandError(cmdArg->argString, arg->argString, res);
        return FALSE;
    }

    if (isDynamicMemory)
    {
        free(filePath);
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
