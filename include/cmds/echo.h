#pragma once
#include <uefi.h>
#include "shellerr.h"

int EchoCmd(char args[], char** currPathPtr);
const char* EchoBrief(void);
const char* EchoLong(void);
