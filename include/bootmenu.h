#pragma once
#include <uefi.h>

typedef struct boot_menu_cfg_s
{
    int32_t maxEntriesOnScreen;
    int32_t selectedEntryIndex;
    int32_t entryOffset;

    int32_t timeoutSeconds;
    boolean_t timeoutCancelled;
    boolean_t bootImmediately;
} boot_menu_cfg_s;

extern boot_menu_cfg_s bmcfg;

void StartBootloader(void);

void PrintBootloaderVersion(void);
void ShowLogFile(void);
