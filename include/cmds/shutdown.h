#pragma once
#include <uefi.h>
#include "shellerr.h"
#include "commanddefs.h"
#include "bootutils.h"

boolean_t ShutdownCmd(cmd_args_s** args, char_t** currPathPtr);
const char_t* ShutdownBrief(void);
