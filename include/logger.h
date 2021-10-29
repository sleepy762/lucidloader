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

int InitLogger(void);
void Log(LogLevel loglevel, efi_status_t status, const char* fmtMessage, ...);
const char* LogLevelString(LogLevel loglevel);
const char* EfiErrorString(efi_status_t status);
time_t GetSecondsSinceInit(void);
