#pragma once
#include <uefi.h>
#include "commanddefs.h"

boolean_t RebootCmd(cmd_args_s** args, char_t** currPathPtr);
const char_t* RebootBrief(void);
const char_t* RebootLong(void);
