#pragma once
#include <uefi.h>
#include "commands.h"
#include "shellutils.h"
#include "bootutils.h"

#define MAX_INPUT (128)

#define CARRIAGE_RETURN ('\r')  // The "character" when you press the enter key
#define BACKSPACE ('\b')
#define SPACE (' ') // Used as a delimiter between a command and the arguments

void StartShell(void);
void ShellLoop(char** currPathPtr);
void ParseInput(char buffer[], char** cmd, char** args);

boolean_t GetInput(char** currPathPtr);
void ProcessCommand(char buffer[], char** currPathPtr);
