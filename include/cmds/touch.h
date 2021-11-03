#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "bootutils.h"
#include "shellerr.h"

uint8_t TouchCmd(char_t args[], char_t** currPathPtr);
const char_t* TouchBrief(void);
const char_t* TouchLong(void);
