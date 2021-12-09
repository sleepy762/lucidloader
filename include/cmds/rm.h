#pragma once
#include <uefi.h>
#include "shellerr.h"
#include "commanddefs.h"
#include "shellutils.h"
#include "bootutils.h"

#define RECURSIVE_FLAG ("-r")

uint8_t RmCmd(cmd_args_s** args, char_t** currPathPtr);
const char_t* RmBrief(void);
const char_t* RmLong(void);

uint8_t RemoveDirRecursively(char_t* mainPath);
