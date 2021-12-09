#include "cmds/clear.h"

uint8_t ClearCmd(cmd_args_s** args, char_t** currPathPtr)
{
    ST->ConOut->ClearScreen(ST->ConOut);
    return CMD_SUCCESS;
}

const char_t* ClearBrief(void)
{
    return "Clears the screen.";
}
