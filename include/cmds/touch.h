#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "bootutils.h"
#include "shellerr.h"

int TouchCmd(char args[], char** currPathPtr);
const char* TouchBrief(void);
const char* TouchLong(void);
