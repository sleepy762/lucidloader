#pragma once
#include <uefi.h>
#include "shell.h"
#include "config.h"
#include "chainloader.h"
#include "bootutils.h"
#include "cmds/cat.h"
#include "logger.h"
#include "version.h"

#define SHELL_CHAR  ('c')
#define INFO_CHAR   ('i')

#define BAD_CONFIGURATION_ERR_MSG ("Configuration file is incorrect or doesn't exist.")
#define FAILED_BOOT_ERR_MSG ("An error has occurred during the booting process.")

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
