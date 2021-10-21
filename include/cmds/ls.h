#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "bootutils.h"
#include "shellerr.h"

int LsCmd(char args[], char** currPathPtr);
const char* LsBrief(void);
const char* LsLong(void);
