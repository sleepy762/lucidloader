#ifndef CHAINLOADER_H
#define CHAINLOADER_H
#include <uefi.h>
#include "debug.h"
#include "bootutils.h"

void ChainloadImage(wchar_t* path);
#endif