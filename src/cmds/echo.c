#include "cmds/echo.h"

uint8_t EchoCmd(cmd_args_s* args, char_t** currPathPtr)
{
    if (args != NULL)
    {
        printf("\n");
        while (args != NULL)
        {
            printf("%s ", args->argString);
            args = args->next;
        }
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
