#include <uefi.h>
#include "shellutils.h"
#include "bootutils.h"

void MkdirCmd(char args[], char** currPathPtr);
const char* MkdirBrief(void);
const char* MkdirLong(void);
