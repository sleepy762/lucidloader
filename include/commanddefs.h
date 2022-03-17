#pragma once
#include <uefi.h>

// Defines a shell command
typedef struct shell_cmd_s
{
    const char_t* commandName;
    boolean_t (*CommandFunction)();
    const char_t* (*BriefHelp)();
    const char_t* (*LongHelp)();
} shell_cmd_s;

// Command arguments struct
typedef struct cmd_args_s
{
    char_t* argString;
    struct cmd_args_s* next;
} cmd_args_s;
