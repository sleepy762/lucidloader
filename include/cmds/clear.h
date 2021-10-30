#pragma once
#include <uefi.h>
#include "shellerr.h"

uint8_t ClearCmd(char_t args[], char_t** currPathPtr);
const char_t* ClearBrief(void);
