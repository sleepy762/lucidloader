#pragma once
#include <uefi.h>

#define LOG_PATH ("\\EFI\\ezboot\\ezboot-log.txt")

#define SECONDS_IN_DAY (86400)
#define SECONDS_IN_HOUR (3600)
#define SECONDS_IN_MINUTE (60)

typedef enum log_level_t
{
    LL_INFO,
    LL_WARNING,
    LL_ERROR
} log_level_t;

extern efi_time_t timeSinceInit;

int8_t InitLogger(void);
void Log(log_level_t loglevel, efi_status_t status, const char_t* fmtMessage, ...);
const char_t* LogLevelString(log_level_t loglevel);
const char_t* EfiErrorString(efi_status_t status);
time_t GetSecondsSinceInit(void);
