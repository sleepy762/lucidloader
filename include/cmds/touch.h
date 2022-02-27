#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "bootutils.h"
#include "shellerr.h"
#include "commanddefs.h"

boolean_t TouchCmd(cmd_args_s** args, char_t** currPathPtr);
const char_t* TouchBrief(void);
const char_t* TouchLong(void);
