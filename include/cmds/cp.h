#pragma once
#include <uefi.h>
#include "commanddefs.h"

boolean_t CpCmd(cmd_args_s** args, char_t** currPathPtr);
const char_t* CpBrief(void);
const char_t* CpLong(void);
