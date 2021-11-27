#include "cmds/reboot.h"

uint8_t RebootCmd(cmd_args_s** args, char_t** currPathPtr)
{
    // Reboot into firmware setup or do a normal cold reboot
    if (*args != NULL && strcmp((*args)->argString, REBOOT_TO_FW) == 0)
    {
        RebootDevice(TRUE);
    }
    else
    {
        RebootDevice(FALSE);
    }
    return CMD_REBOOT_FAIL;
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
