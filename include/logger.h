#pragma once
#include <uefi.h>

typedef enum log_level_t
{
    LL_INFO,
    LL_WARNING,
    LL_ERROR
} log_level_t;

int8_t InitLogger(void);
void Log(log_level_t loglevel, efi_status_t status, const char_t* fmtMessage, ...);
const char_t* LogLevelString(log_level_t loglevel);
const char_t* EfiErrorString(efi_status_t status);
time_t GetSecondsSinceInit(void);

void PrintLogFile(void);
