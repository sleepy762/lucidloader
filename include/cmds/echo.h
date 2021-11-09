#pragma once
#include <uefi.h>
#include "shellerr.h"
#include "commanddefs.h"

uint8_t EchoCmd(cmd_args_s* args, char_t** currPathPtr);
const char_t* EchoBrief(void);
const char_t* EchoLong(void);
