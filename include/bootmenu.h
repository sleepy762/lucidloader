#pragma once
#include <uefi.h>
#include "shell.h"
#include "config.h"
#include "chainloader.h"
#include "bootutils.h"
#include "cmds/cat.h"
#include "logger.h"

#define CHAR_INT 48 // to convert from unicode to regular numbers
#define SHELL_CHAR 'c'

void MainMenu(void);
void SuccessMenu(boot_entry_s* head);
void FailMenu(void);
void ShowLogFile(void);
boot_entry_s * GetCurrOS(uint8_t numOfPartition, boot_entry_s * head);
