#include "cmds/pwd.h"

int PwdCmd(char args[], char** currPathPtr)
{
    printf("\n%s", *currPathPtr);
    return CMD_SUCCESS;
}

const char* PwdBrief(void)
{
    return "Print the current working directory.";
}
