#include "cmds/shutdown.h"

uint8_t ShutdownCmd(cmd_args_s* args, char_t** currPathPtr)
{
    ShutdownDevice();
    return CMD_SHUTDOWN_FAIL;
}

const char_t* ShutdownBrief(void)
{
    return "Shuts down the device.";
}
