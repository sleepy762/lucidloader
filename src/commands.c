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
{ "", NULL, NULL, NULL } // Has to be here in order to terminate the command counter
};

// Count the amount of commands
short CommandCount(void)
{
    short totalCmds = 0;
    while (commands[totalCmds].CommandFunction != NULL) 
        totalCmds++;
    return totalCmds;
}

void HelpCmd(char args[], char** currPathPtr)
{
    short totalCmds = CommandCount();

    if (strlen(args) != 0)
    {
        // If an arg(command name) was passed, find help for it
        for (short i = 0; i < totalCmds; i++)
        {
            if (strcmp(args, commands[i].commandName) == 0)
            {
                if (commands[i].LongHelp != NULL)
                    printf("\n%s", commands[i].LongHelp());
                else
                    printf("\nNo long help available.");
                return; // The command was found
            }
        }
        printf("\nhelp: Command '%s' not found.", args);
    }
    else
    {
        // List all commands with their brief help
        for (short i = 0; i < totalCmds; i++)
        {
            printf("\n%s -- ", commands[i].commandName);

            if (commands[i].BriefHelp != NULL)
                printf("%s", commands[i].BriefHelp());
            else
                printf("No brief help available.\n");
        }
    }
}

const char* HelpBrief(void)
{
    return "Displays all the commands and their description. help [cmd] for info on a command.";
}

const char* HelpLong(void)
{
    return "Usage: help [cmd]\n[cmd] - Optional argument. When passed, it will look for the command \
and print long help information.";
}
