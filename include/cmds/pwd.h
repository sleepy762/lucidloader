#pragma once
#include <uefi.h>
#include "shellerr.h"

uint8_t PwdCmd(char_t args[], char_t** currPathPtr);
const char_t* PwdBrief(void);
