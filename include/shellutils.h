#pragma once
#include <uefi.h>
#include "debug.h"
#include "bootutils.h"
#include "shellerr.h"

#define DIRECTORY_DELIM ('\\')
#define DIRECTORY_DELIM_STR ("\\")
#define CURRENT_DIR (".")
#define PREVIOUS_DIR ("..")

char* ConcatPaths(char* lhs, char* rhs);
boolean_t isspace(char c);
void RemoveRepeatedChars(char* str, char toRemove);
int NormalizePath(char** path);
char* TrimSpaces(char* str);
void CleanPath(char** path);
char* MakeFullPath(char* args, char* currPathPtr, boolean_t* dynMemFlag);
