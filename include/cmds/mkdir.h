#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "bootutils.h"
#include "shellerr.h"
#include "commanddefs.h"

uint8_t MkdirCmd(cmd_args_s* args, char_t** currPathPtr);
const char_t* MkdirBrief(void);
const char_t* MkdirLong(void);
