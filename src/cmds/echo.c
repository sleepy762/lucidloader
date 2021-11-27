#include "cmds/echo.h"

uint8_t EchoCmd(cmd_args_s* args, char_t** currPathPtr)
{
    if (args != NULL)
    {
        while (args != NULL)
        {
            printf("%s ", args->argString);
            args = args->next;
        }
        printf("\n"); // We want this to be printed once, only if there are args
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
