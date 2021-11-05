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
#include "cmds/clear.h"
#include "cmds/cat.h"

// Defines a shell command
typedef struct shell_cmd_s
{
    const char_t* commandName;
    uint8_t (*CommandFunction)();
    const char_t* (*BriefHelp)();
    const char_t* (*LongHelp)();
} shell_cmd_s;

// The only command in this file because it has to access the list of all commands
uint8_t HelpCmd(char_t args[], char_t** currPathPtr);
const char_t* HelpBrief(void);
const char_t* HelpLong(void);

uint8_t CommandCount(void);

extern const shell_cmd_s commands[];
