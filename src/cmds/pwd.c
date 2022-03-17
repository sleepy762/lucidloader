#include "cmds/pwd.h"

boolean_t PwdCmd(cmd_args_s** args, char_t** currPathPtr)
{
    printf("%s\n", *currPathPtr);
    return TRUE;
}

const char_t* PwdBrief(void)
{
    return "Print the current working directory.";
}
