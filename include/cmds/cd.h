#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "bootutils.h"
#include "shellerr.h"
#include "commanddefs.h"

uint8_t CdCmd(cmd_args_s* args, char_t** currPathPtr);
const char_t* CdBrief(void);
const char_t* CdLong(void);
