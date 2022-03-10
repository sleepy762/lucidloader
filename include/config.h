#pragma once
#include <uefi.h>
#include "logger.h"
#include "bootutils.h"
#include "shellutils.h"

// Entries config path
#define CFG_PATH ("EFI\\ezboot\\ezboot-entries.cfg")

#define CFG_LINE_DELIMITER ("\n")
#define CFG_ENTRY_DELIMITER ("\n\n")
#define CFG_KEY_VALUE_DELIMITER ('=')

#define MAX_ENTRY_NAME_LEN (70)

typedef struct boot_entry_s
{
    char_t* name; // Name in the menu
    char_t* mainPath; // Holds a path to the file to load
    char_t* imgArgs; // Used if the image needs args
} boot_entry_s;

typedef struct boot_entry_array_s
{
    boot_entry_s* entries;
    int32_t numOfEntries;
} boot_entry_array_s;

boot_entry_array_s ParseConfig(void);
void FreeConfigEntries(boot_entry_array_s* entryArr);
