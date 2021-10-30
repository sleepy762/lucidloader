#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "bootutils.h"
#include "shellerr.h"

uint8_t LsCmd(char_t args[], char_t** currPathPtr);
const char_t* LsBrief(void);
const char_t* LsLong(void);
