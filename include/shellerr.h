#pragma once
#include <uefi.h>

#define CMD_SUCCESS (0)
#define CMD_NO_FILE_SPECIFIED (35)
#define CMD_NO_DIR_SPEFICIED (36)
#define CMD_GENERAL_FILE_OPENING_ERROR (37)
#define CMD_GENERAL_DIR_OPENING_ERROR (38)
#define CMD_DIR_ALREADY_EXISTS (39)
#define CMD_DIR_NOT_FOUND (40)
#define CMD_CANT_READ_DIR (41)
#define CMD_OUT_OF_MEMORY (42)
#define CMD_NOT_FOUND (43)
#define CMD_BRIEF_HELP_NOT_AVAILABLE (44)
#define CMD_LONG_HELP_NOT_AVAILABLE (45)
#define CMD_QUOTATION_MARK_OPEN (46)

const char_t* GetCommandErrorInfo(const uint8_t error);
void PrintCommandError(const char_t* cmd, const char_t* args, const uint8_t error);
