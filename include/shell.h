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

#define SHELL_EXIT_STR ("exit")

int8_t StartShell(void);
int8_t ShellLoop(char_t** currPathPtr);
int8_t ParseInput(char_t buffer[], char_t** cmd, char_t** args);

void GetInput(char_t buffer[], const uint32_t maxInputSize);
int8_t ProcessCommand(char_t buffer[], char_t** currPathPtr);
