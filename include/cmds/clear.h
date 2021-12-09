#pragma once
#include <uefi.h>
#include "shellerr.h"
#include "commanddefs.h"

uint8_t ClearCmd(cmd_args_s** args, char_t** currPathPtr);
const char_t* ClearBrief(void);
