#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "shellerr.h"

uint8_t CatCmd(char_t args[], char_t** currPathPtr);
const char_t* CatBrief(void);
const char_t* CatLong(void);
