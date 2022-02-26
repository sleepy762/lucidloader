#include "cmds/edit.h"

uint8_t EditCmd(cmd_args_s** args, char_t** currPathPtr)
{
    boolean_t isDynamicMemory = FALSE;

    char_t* filePath = NULL;
    if (*args != NULL)
    {
        filePath = MakeFullPath((*args)->argString, *currPathPtr, &isDynamicMemory);
        if (filePath == NULL)
        {
            return CMD_NO_FILE_SPECIFIED;
        }
    }

    int8_t res = StartEditor(filePath);

    if (isDynamicMemory)
    {
        BS->FreePool(filePath);
    }

    return res;
}

const char_t* EditBrief(void)
{
    return "Text editor.";
}

const char_t* EditLong(void)
{
    return "Usage: edit [filename]";
}
