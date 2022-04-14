#pragma once
#include <uefi.h>

// Global console size variables
// These variables should be updated ONLY in screen.c functions
extern uintn_t screenRows;
extern uintn_t screenCols;

// A flag that when set to TRUE means that the console size is known and safe to use
extern boolean_t screenModeSet;

boolean_t SetMaxConsoleSize(void);
boolean_t QueryCurrentConsoleSize(void);
void PrepareScreenForRedraw(void);
