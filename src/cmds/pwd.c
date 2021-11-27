#include "cmds/pwd.h"

uint8_t PwdCmd(cmd_args_s* args, char_t** currPathPtr)
{
    printf("\n%s", *currPathPtr);
    return CMD_SUCCESS;
}

const char_t* PwdBrief(void)
{
    return "Print the current working directory.";
}
