#pragma once
#include <uefi.h>
#include "shellerr.h"
#include "bootutils.h"
#include "commanddefs.h"

boolean_t PwdCmd(cmd_args_s** args, char_t** currPathPtr);
const char_t* PwdBrief(void);
