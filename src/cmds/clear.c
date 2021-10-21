#include "cmds/clear.h"

int ClearCmd(char args[], char** currPathPtr)
{
    ST->ConOut->ClearScreen(ST->ConOut);
    return CMD_SUCCESS;
}

const char* ClearBrief(void)
{
    return "Clears the screen.";
}
