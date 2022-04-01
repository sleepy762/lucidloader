#pragma once
#include <uefi.h>
#include "commanddefs.h"

boolean_t MkdirCmd(cmd_args_s** args, char_t** currPathPtr);
const char_t* MkdirBrief(void);
const char_t* MkdirLong(void);
