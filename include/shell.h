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
