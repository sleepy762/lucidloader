#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "bootutils.h"

void TouchCmd(char args[], char** currPathPtr);
const char* TouchBrief(void);
const char* TouchLong(void);
