#include <uefi.h>
#include "shellutils.h"
#include "bootutils.h"
#include "shellerr.h"

int MkdirCmd(char args[], char** currPathPtr);
const char* MkdirBrief(void);
const char* MkdirLong(void);
