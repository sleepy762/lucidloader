#pragma once
#include <uefi.h>
#include "shellerr.h"

uint8_t EchoCmd(char_t args[], char_t** currPathPtr);
const char_t* EchoBrief(void);
const char_t* EchoLong(void);
