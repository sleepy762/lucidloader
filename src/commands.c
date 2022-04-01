#include "commands.h"
#include "shellutils.h"
#include "shellerr.h"
#include "bootutils.h"
#include "commanddefs.h"

#include "cmds/pwd.h"
#include "cmds/ls.h"
#include "cmds/cd.h"
#include "cmds/touch.h"
#include "cmds/mkdir.h"
#include "cmds/clear.h"
#include "cmds/cat.h"
#include "cmds/reboot.h"
#include "cmds/shutdown.h"
#include "cmds/rm.h"
#include "cmds/edit.h"
#include "cmds/passwd.h"
#include "cmds/cp.h"
#include "cmds/about.h"

// List of all the commands
const shell_cmd_s commands[] = {
{ "help",     HelpCmd,     HelpBrief,     HelpLong },
{ "pwd",      PwdCmd,      PwdBrief,      NULL },
{ "ls",       LsCmd,       LsBrief,       LsLong },
{ "cd",       CdCmd,       CdBrief,       CdLong },
{ "touch",    TouchCmd,    TouchBrief,    TouchLong },
{ "mkdir",    MkdirCmd,    MkdirBrief,    MkdirLong },
{ "clear",    ClearCmd,    ClearBrief,    NULL },
{ "cat",      CatCmd,      CatBrief,      CatLong },
{ "reboot",   RebootCmd,   RebootBrief,   RebootLong },
{ "shutdown", ShutdownCmd, ShutdownBrief, NULL },
{ "rm",       RmCmd,       RmBrief,       RmLong },
{ "edit",     EditCmd,     EditBrief,     EditLong },
{ "passwd",   PasswdCmd,   PasswdBrief,   PasswdLong },
{ "cp",       CpCmd,       CpBrief,       CpLong },
{ "about",    AboutCmd,    AboutBrief,    NULL },
{ "", NULL, NULL, NULL } // Has to be here in order to terminate the command counter
};


// Count the amount of commands
uint8_t CommandCount(void)
{
    uint8_t totalCmds = 0;
    while (commands[totalCmds].CommandFunction != NULL)
    {
        totalCmds++;
    }
        
    return totalCmds;
}

boolean_t HelpCmd(cmd_args_s** args, char_t** currPathPtr)
{
    uint8_t totalCmds = CommandCount();

    cmd_args_s* cmdArg = *args;
    cmd_args_s* arg = cmdArg->next;

    // This command only uses the first argument
    if (arg != NULL)
    {
        // If an arg(command name) was passed, find help for it
        for (uint8_t i = 0; i < totalCmds; i++)
        {
            if (strcmp(arg->argString, commands[i].commandName) == 0)
            {
                if (commands[i].LongHelp != NULL)
                {
                    printf("%s\n", commands[i].LongHelp());
                }
                else
                {
                    PrintCommandError(cmdArg->argString, arg->argString, CMD_LONG_HELP_NOT_AVAILABLE);
                    return FALSE;
                }
                return TRUE; // The command was found and we can leave
            }
        }
        PrintCommandError(cmdArg->argString, arg->argString, CMD_NOT_FOUND);
        return FALSE;
    }
    else
    {
        // List all commands with their brief help
        for (uint8_t i = 0; i < totalCmds; i++)
        {
            printf("%s -- ", commands[i].commandName);

            if (commands[i].BriefHelp != NULL)
            {
                printf("%s", commands[i].BriefHelp());
            }
            else
            {
                printf("no description available.");
            }
            putchar('\n');
        }
    }
    return TRUE;
}

const char_t* HelpBrief(void)
{
    return "Displays all the commands and their description.";
}

const char_t* HelpLong(void)
{
    return "Usage: help [cmd]\n[cmd] - Optional argument. When passed, it will look for the command \
and print long help information.";
}
