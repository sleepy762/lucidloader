#pragma once
#include <uefi.h>
#include "shellerr.h"

int ClearCmd(char args[], char** currPathPtr);
const char* ClearBrief(void);
