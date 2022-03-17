#pragma once
#include <uefi.h>
#include "logger.h"
#include "bootutils.h"

void ChainloadImage(char_t* path, char_t* args);
