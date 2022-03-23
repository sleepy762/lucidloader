#include "cmds/shutdown.h"
#include "shellerr.h"
#include "bootutils.h"

boolean_t ShutdownCmd(cmd_args_s** args, char_t** currPathPtr)
{
    ShutdownDevice();

    cmd_args_s* cmdArg = *args;
    PrintCommandError(cmdArg->argString, NULL, CMD_SHUTDOWN_FAIL);
    return FALSE;
}

const char_t* ShutdownBrief(void)
{
    return "Shuts down the device.";
}
