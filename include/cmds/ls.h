#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "bootutils.h"
#include "shellerr.h"
#include "commanddefs.h"

boolean_t LsCmd(cmd_args_s** args, char_t** currPathPtr);
const char_t* LsBrief(void);
const char_t* LsLong(void);
