#include "cmds/echo.h"

void EchoCmd(char args[], char** currPathPtr)
{
    if (args) 
        printf("\n%s", args);
}

const char* EchoBrief(void)
{
    return "Print a string. Used for debugging.";
}

const char* EchoLong(void)
{
    return "Usage: echo <string>";
}
