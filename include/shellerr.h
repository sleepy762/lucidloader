#pragma once
#include <uefi.h>

#define CMD_SUCCESS (0)
#define CMD_NO_FILE_SPECIFIED (1)
#define CMD_NO_DIR_SPEFICIED (2)
#define CMD_GENERAL_FILE_OPENING_ERROR (3)
#define CMD_GENERAL_DIR_OPENING_ERROR (4)
#define CMD_READ_ONLY_FILESYSTEM (5)
#define CMD_DIR_ALREADY_EXISTS (6)
#define CMD_DIR_NOT_FOUND (7)
#define CMD_CANT_READ_DIR (8)
#define CMD_OUT_OF_MEMORY (9)
#define CMD_NOT_FOUND (10)
#define CMD_BRIEF_HELP_NOT_AVAILABLE (11)
#define CMD_LONG_HELP_NOT_AVAILABLE (12)

const char_t* GetCommandErrorInfo(uint8_t error);
void PrintCommandError(const char_t* cmd, uint8_t error);
