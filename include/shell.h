#pragma once
#include <uefi.h>
#include "commands.h"
#include "shellutils.h"
#include "bootutils.h"
#include "shellerr.h"

#define SHELL_MAX_INPUT (128)

#define CARRIAGE_RETURN ('\r')  // The "character" when you press the enter key
#define BACKSPACE ('\b')
#define SPACE (' ') // Used as a delimiter between a command and the arguments
#define QUOTATION_MARK ('"')

#define SHELL_EXIT_STR ("exit")

int8_t StartShell(void);
int8_t ShellLoop(char_t** currPathPtr);

void ParseInput(char_t buffer[], char_t** cmd, char_t** args);
int8_t ParseArgs(char_t* inputArgs, cmd_args_s** outputArgs);
int8_t SplitArgsString(char_t buffer[], cmd_args_s** outputArgs);
cmd_args_s* InitializeArgsNode(void);
void AppendArgsNode(cmd_args_s* head, cmd_args_s* node);
void FreeArgs(cmd_args_s* args);

void GetInput(char_t buffer[], const uint32_t maxInputSize);
int8_t ProcessCommand(char_t buffer[], char_t** currPathPtr);
