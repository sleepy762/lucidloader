#pragma once
#include <uefi.h>

void EchoCmd(char args[], char** currPathPtr);
const char* EchoBrief(void);
const char* EchoLong(void);
