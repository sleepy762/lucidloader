#pragma once
#include <uefi.h>

extern uintn_t screenRows;
extern uintn_t screenCols;

boolean_t SetMaxConsoleSize(void);
