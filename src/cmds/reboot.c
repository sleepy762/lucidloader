#include "cmds/reboot.h"

boolean_t RebootCmd(cmd_args_s** args, char_t** currPathPtr)
{
    cmd_args_s* cmdArg = *args;
    cmd_args_s* arg = cmdArg->next;
    // Reboot into firmware setup or do a normal reboot
    if (arg != NULL && strcmp(arg->argString, REBOOT_TO_FW) == 0)
    {
        RebootDevice(TRUE);
    }
    else
    {
        RebootDevice(FALSE);
    }

    PrintCommandError(cmdArg->argString, NULL, CMD_REBOOT_FAIL);
    return FALSE;
}

const char_t* RebootBrief(void)
{
    return "Reboot the device.";
}

const char_t* RebootLong(void)
{
    return "Usage: reboot [fw]\n\
Passing `fw` as a parameter will cause the device to reboot into firmware settings.";
}
