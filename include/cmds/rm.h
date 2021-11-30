#pragma once
#include <uefi.h>
#include "shellerr.h"
#include "commanddefs.h"
#include "shellutils.h"
#include "bootutils.h"

extern int __remove (const char_t *__filename, int isdir);
#define RemoveFile(filePath) (__remove(filePath, -1))
#define RemoveDir(dirPath) (__remove(dirPath, 1))

uint8_t RmCmd(cmd_args_s** args, char_t** currPathPtr);
const char_t* RmBrief(void);
const char_t* RmLong(void);

uint8_t RemoveDirRecursively(char_t* mainPath);
