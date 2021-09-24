#pragma once
#include <uefi.h>
#include "debug.h"

#define FALSE (boolean_t)0
#define TRUE (boolean_t)1

#define DIRECTORY_DELIM '\\'
#define DIRECTORY_DELIM_STR "\\"
#define CURRENT_DIR "."
#define PREVIOUS_DIR ".."

wchar_t* StringToWideString(char* str);

void GetFileProtocols(wchar_t* path, efi_device_path_t** devPath, efi_file_handle_t** rootDir, efi_file_handle_t** imgFileHandle);
efi_status_t GetFileInfo(efi_file_handle_t* fileHandle, efi_file_info_t* fileInfo);
efi_status_t ReadFile(efi_file_handle_t* fileHandle, uintn_t fileSize, char** buffer);
int GetValueOffset(char* line, size_t* valueOffset, const char delimiter);

char* ConcatPaths(char* lhs, char* rhs);
boolean_t isspace(char c);
void RemoveRepeatedChars(char* str, char toRemove);
void NormalizePath(char** path);
void CleanPath(char** path);