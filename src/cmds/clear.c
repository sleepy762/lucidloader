#include "cmds/clear.h"

boolean_t ClearCmd(cmd_args_s** args, char_t** currPathPtr)
{
    ST->ConOut->ClearScreen(ST->ConOut);
    return TRUE;
}

const char_t* ClearBrief(void)
{
    return "Clears the screen.";
}
