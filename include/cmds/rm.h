#pragma once
#include <uefi.h>
#include "commanddefs.h"

boolean_t RmCmd(cmd_args_s** args, char_t** currPathPtr);
const char_t* RmBrief(void);
const char_t* RmLong(void);
