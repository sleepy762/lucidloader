#include "commands.h"

// Count the amount of commands
short CommandCount(void)
{
    short totalCmds = 0;
    while(commands[totalCmds].CommandFunction) totalCmds++;
    return totalCmds;
}

void HelpCmd(char args[])
{
    short totalCmds = CommandCount();

    if (strlen(args))
    {
        // If an arg(command name) was passed, find help for it
        for(short i = 0; i < totalCmds; i++)
        {
            if(strcmp(args, commands[i].commandName) == 0)
            {
                printf("\n%s\n", commands[i].LongHelp());
                return; // The command was found
            }
        }
        printf("\nCommand '%s' not found.", args);
    }
    else
    {
        // List all commands with their brief help
        for(short i = 0; i < totalCmds; i++)
        {
            printf("\n%s -- %s\n", commands[i].commandName, commands[i].BriefHelp());
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