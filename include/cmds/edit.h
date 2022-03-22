#pragma once
#include <uefi.h>
#include "commanddefs.h"

boolean_t EditCmd(cmd_args_s** args, char_t** currPathPtr);
const char_t* EditBrief(void);
const char_t* EditLong(void);
