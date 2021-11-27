#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "bootutils.h"
#include "shellerr.h"
#include "commanddefs.h"

uint8_t LsCmd(cmd_args_s** args, char_t** currPathPtr);
const char_t* LsBrief(void);
const char_t* LsLong(void);

uint8_t ListDir(char_t* path);
