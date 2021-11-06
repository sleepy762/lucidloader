#pragma once
#include <uefi.h>
#include "logger.h"
#include "bootutils.h"
#include "shellerr.h"

#define DIRECTORY_DELIM ('\\')
#define DIRECTORY_DELIM_STR ("\\")
#define CURRENT_DIR (".")
#define PREVIOUS_DIR ("..")

char_t* ConcatPaths(char_t* lhs, char_t* rhs);
uint8_t NormalizePath(char_t** path);
void CleanPath(char_t** path);
char_t* MakeFullPath(char_t* args, char_t* currPathPtr, boolean_t* isDynamicMemory);

efi_input_key_t GetKey(void);
boolean_t isspace(char_t c);
char_t* TrimSpaces(char_t* str);
void RemoveRepeatedChars(char_t* str, char_t toRemove);
int32_t GetValueOffset(char_t* line, const char_t delimiter);
