#pragma once
#include <uefi.h>
#include "commanddefs.h"

boolean_t CdCmd(cmd_args_s** args, char_t** currPathPtr);
const char_t* CdBrief(void);
const char_t* CdLong(void);
