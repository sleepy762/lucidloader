#pragma once
#include <uefi.h>
#include "logger.h"
#include "bootutils.h"

efi_status_t ChainloadImage(char_t* path);
