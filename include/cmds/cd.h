#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "bootutils.h"
#include "shellerr.h"

uint8_t CdCmd(char_t args[], char_t** currPathPtr);
const char_t* CdBrief(void);
const char_t* CdLong(void);
