#include "cmds/edit.h"

uint8_t EditCmd(cmd_args_s** args, char_t** currPathPtr)
{
    return StartEditor();
}

const char_t* EditBrief(void)
{
    return "Text editor.";
}

const char_t* EditLong(void)
{
    return "Usage: edit [filename]";
}
