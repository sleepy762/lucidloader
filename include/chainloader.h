#pragma once
#include <uefi.h>
#include "logger.h"
#include "bootutils.h"

int ChainloadImage(wchar_t* path);
