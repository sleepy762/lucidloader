#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "bootutils.h"
#include "shellerr.h"

uint8_t MkdirCmd(char_t args[], char_t** currPathPtr);
const char_t* MkdirBrief(void);
const char_t* MkdirLong(void);
