#pragma once
#include <uefi.h>
#include "debug.h"
#include "bootutils.h"

#define DIRECTORY_DELIM ('\\')
#define DIRECTORY_DELIM_STR ("\\")
#define CURRENT_DIR (".")
#define PREVIOUS_DIR ("..")

char* ConcatPaths(char* lhs, char* rhs);
boolean_t isspace(char c);
void RemoveRepeatedChars(char* str, char toRemove);
void NormalizePath(char** path);
void CleanPath(char** path);
char* MakeFullPath(char* args, char* currPathPtr, boolean_t* dynMemFlag);
