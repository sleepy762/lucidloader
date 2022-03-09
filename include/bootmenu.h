#pragma once
#include <uefi.h>
#include "shell.h"
#include "config.h"
#include "chainloader.h"
#include "bootutils.h"
#include "cmds/cat.h"
#include "logger.h"
#include "version.h"

#define SHELL_CHAR 'c'

#define BAD_CONFIGURATION_ERR_MSG ("Configuration file is incorrect or doesn't exist.")
#define FAILED_BOOT_ERR_MSG ("An error has occurred during the booting process.")

void MainMenu(void);

void PrintBootloaderVersion(void);
void ShowLogFile(void);
