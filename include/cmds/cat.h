#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "shellerr.h"
#include "commanddefs.h"

boolean_t CatCmd(cmd_args_s** args, char_t** currPathPtr);
const char_t* CatBrief(void);
const char_t* CatLong(void);
