#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "shellerr.h"
#include "commanddefs.h"

#include "cmds/echo.h"
#include "cmds/pwd.h"
#include "cmds/ls.h"
#include "cmds/cd.h"
#include "cmds/touch.h"
#include "cmds/mkdir.h"
#include "cmds/clear.h"
#include "cmds/cat.h"

// The only command in this file because it has to access the list of all commands
uint8_t HelpCmd(cmd_args_s* args, char_t** currPathPtr);
const char_t* HelpBrief(void);
const char_t* HelpLong(void);

uint8_t CommandCount(void);

extern const shell_cmd_s commands[];
