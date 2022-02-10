#pragma once
#include <uefi.h>
#include "commanddefs.h"
#include "password.h"
#include "encryption.h"
#include "shellerr.h"
#include "shellutils.h"

#define MAX_PASS_LEN 16

uint8_t PasswdCmd(cmd_args_s** args, char_t** currPathPtr);
const char_t* PasswdBrief(void);
const char_t* PasswdLong(void);
