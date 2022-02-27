#pragma once
#include <uefi.h>
#include "shellerr.h"
#include "commanddefs.h"
#include "bootutils.h"

// The argument that the user must pass in order to enter firmware settings
#define REBOOT_TO_FW ("fw")

boolean_t RebootCmd(cmd_args_s** args, char_t** currPathPtr);
const char_t* RebootBrief(void);
const char_t* RebootLong(void);
