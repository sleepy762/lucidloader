#pragma once
#include <uefi.h>
#include "shellerr.h"

int PwdCmd(char args[], char** currPathPtr);
const char* PwdBrief(void);
