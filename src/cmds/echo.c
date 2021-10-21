#include "cmds/echo.h"

int EchoCmd(char args[], char** currPathPtr)
{
    if (args != NULL)
    {
        printf("\n%s", args);
    }
    return CMD_SUCCESS;
}

const char* EchoBrief(void)
{
    return "Print a string. Used for debugging.";
}

const char* EchoLong(void)
{
    return "Usage: echo <string>";
}
