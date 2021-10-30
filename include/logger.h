#pragma once
#include <uefi.h>

#define LOG_PATH ("\\EFI\\apps\\ezboot-log.txt")

#define SECONDS_IN_DAY (86400)
#define SECONDS_IN_HOUR (3600)
#define SECONDS_IN_MINUTE (60)

typedef enum LogLevel
{
    LL_INFO,
    LL_WARNING,
    LL_ERROR
} LogLevel;

extern efi_time_t timeSinceInit;

int8_t InitLogger(void);
void Log(LogLevel loglevel, efi_status_t status, const char_t* fmtMessage, ...);
const char_t* LogLevelString(LogLevel loglevel);
const char_t* EfiErrorString(efi_status_t status);
time_t GetSecondsSinceInit(void);
