#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "bootutils.h"

void CdCmd(char args[], char** currPathPtr);
const char* CdBrief(void);
const char* CdLong(void);
