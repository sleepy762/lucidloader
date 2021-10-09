#include "cmds/pwd.h"

void PwdCmd(char args[], char** currPathPtr)
{
    printf("\n%s", *currPathPtr);
}

const char* PwdBrief(void)
{
    return "Print the current working directory.";
}
