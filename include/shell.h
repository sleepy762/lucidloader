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

int StartShell(void);
int ShellLoop(char** currPathPtr);
int ParseInput(char buffer[], char** cmd, char** args);

void GetInput(char buffer[], const int maxInputSize);
int ProcessCommand(char buffer[], char** currPathPtr);
