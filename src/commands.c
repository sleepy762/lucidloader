#include "commands.h"

// List of all the commands
const shell_cmd_s commands[] = {
{ "help", &HelpCmd, &HelpBrief, &HelpLong },
{ "echo", &EchoCmd, &EchoBrief, &EchoLong },
{ "pwd", &PwdCmd, &PwdBrief, NULL },
{ "ls", &LsCmd, &LsBrief, &LsLong },
{ "cd", &CdCmd, &CdBrief, &CdLong },
{ "touch", &TouchCmd, &TouchBrief, &TouchLong },
{ "mkdir", &MkdirCmd, &MkdirBrief, &MkdirLong },
{ "clear", &ClearCmd, &ClearBrief, NULL },
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

uint8_t HelpCmd(char_t args[], char_t** currPathPtr)
{
    uint8_t totalCmds = CommandCount();

    if (strlen(args) != 0)
    {
        // If an arg(command name) was passed, find help for it
        for (uint8_t i = 0; i < totalCmds; i++)
        {
            if (strcmp(args, commands[i].commandName) == 0)
            {
                if (commands[i].LongHelp != NULL)
                {
                    printf("\n%s", commands[i].LongHelp());
                }
                else
                {
                    return CMD_LONG_HELP_NOT_AVAILABLE;
                }
                return CMD_SUCCESS; // The command was found and we can leave
            }
        }
        return CMD_NOT_FOUND;
    }
    else
    {
        // List all commands with their brief help
        for (uint8_t i = 0; i < totalCmds; i++)
        {
            printf("\n%s -- ", commands[i].commandName);

            if (commands[i].BriefHelp != NULL)
            {
                printf("%s", commands[i].BriefHelp());
            }
            else
            {
                return CMD_BRIEF_HELP_NOT_AVAILABLE;
            }
        }
    }
    return CMD_SUCCESS;
}

const char_t* HelpBrief(void)
{
    return "Displays all the commands and their description. help [cmd] for info on a command.";
}

const char_t* HelpLong(void)
{
    return "Usage: help [cmd]\n[cmd] - Optional argument. When passed, it will look for the command \
and print long help information.";
}
