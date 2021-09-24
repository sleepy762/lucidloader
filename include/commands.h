#pragma once
#include <uefi.h>
#include "bootutils.h"
#include "cmds/echo.h"
#include "cmds/pwd.h"
#include "cmds/ls.h"
#include "cmds/cd.h"
#include "cmds/touch.h"

// Defines a shell command
typedef struct shell_cmd_s
{
    const char* commandName;
    void (*CommandFunction)();
    const char* (*BriefHelp)();
    const char* (*LongHelp)();
}shell_cmd_s;

// The only command in this file because it has to access the list of all commands
void HelpCmd(char args[], char** currPathPtr);
const char* HelpBrief(void);
const char* HelpLong(void);

short CommandCount(void);

// List of all the commands
static shell_cmd_s commands[] = {
{ "help", &HelpCmd, &HelpBrief, &HelpLong },
{ "echo", &EchoCmd, &EchoBrief, &EchoLong },
{ "pwd", &PwdCmd, &PwdBrief, NULL },
{ "ls", &LsCmd, &LsBrief, &LsLong },
{ "cd", &CdCmd, &CdBrief, &CdLong },
{ "touch", &TouchCmd, &TouchBrief, &TouchLong },
{ "", NULL, NULL, NULL } // Has to be here in order to terminate the command counter
};
