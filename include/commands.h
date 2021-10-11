#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "shellerr.h"

#include "cmds/echo.h"
#include "cmds/pwd.h"
#include "cmds/ls.h"
#include "cmds/cd.h"
#include "cmds/touch.h"
#include "cmds/mkdir.h"

// Defines a shell command
typedef struct shell_cmd_s
{
    const char* commandName;
    int (*CommandFunction)();
    const char* (*BriefHelp)();
    const char* (*LongHelp)();
} shell_cmd_s;

// The only command in this file because it has to access the list of all commands
int HelpCmd(char args[], char** currPathPtr);
const char* HelpBrief(void);
const char* HelpLong(void);

short CommandCount(void);

extern const shell_cmd_s commands[];
