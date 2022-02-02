#pragma once
#include <uefi.h>
#include "shell.h"
#include "config.h"
#include "chainloader.h"
#include "bootutils.h"
#include "cmds/cat.h"
#include "logger.h"
#include "version.h"

#define CHAR_INT 48 // to convert from unicode to regular numbers
#define SHELL_CHAR 'c'

#define BAD_CONFIGURATION_ERR_MSG ("Configuration file is incorrect or doesn't exist.")
#define FAILED_BOOT_ERR_MSG ("An error has occurred during the booting process.")

void PrintBootloaderVersion(void);
void MainMenu(void);
void SuccessMenu(boot_entry_s* head);
void FailMenu(const char_t* errorMsg);
void ShowLogFile(void);
boot_entry_s * GetCurrOS(uint8_t numOfPartition, boot_entry_s * head);
