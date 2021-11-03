#include "cmds/echo.h"

uint8_t EchoCmd(char_t args[], char_t** currPathPtr)
{
    if (args != NULL)
    {
        printf("\n%s", args);
    }
    return CMD_SUCCESS;
}

const char_t* EchoBrief(void)
{
    return "Print a string. Used for debugging.";
}

const char_t* EchoLong(void)
{
    return "Usage: echo <string>";
}
