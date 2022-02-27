#pragma once
#include <uefi.h>
#include "commands.h"
#include "shellutils.h"
#include "bootutils.h"
#include "shellerr.h"

#define SHELL_MAX_INPUT (128)

#define SPACE (' ') // Used as a delimiter between a command and the arguments
#define QUOTATION_MARK ('"')

#define SHELL_EXIT_STR ("exit")

int8_t StartShell(void);
int8_t ShellLoop(char_t** currPathPtr);

char_t* GetCommandFromBuffer(char_t buffer[]);

int8_t ParseArgs(char_t* inputArgs, cmd_args_s** outputArgs);
int8_t SplitArgsString(char_t buffer[], cmd_args_s** outputArgs);
cmd_args_s* InitializeArgsNode(void);
void AppendArgsNode(cmd_args_s* head, cmd_args_s* node);
void FreeArgs(cmd_args_s* args);

int8_t ProcessCommand(char_t buffer[], char_t** currPathPtr);
