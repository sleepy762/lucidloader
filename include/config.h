#pragma once
#include <uefi.h>
#include "logger.h"
#include "bootutils.h"
#include "shellutils.h"

// Temporary path
#define CFG_PATH ("EFI\\ezboot\\config.cfg")
#define CFG_LINE_DELIMITER ("\n")
#define CFG_ENTRY_DELIMITER ("\n\n")
#define CFG_KEY_VALUE_DELIMITER ('=')

// General struct which applies to any type of booting
// The mainPath variable depends on the boot type -
// When chainloading, it holds the path to the image to be chainloaded
// With Linux, it holds the path to the kernel to be loaded
typedef struct boot_entry_s
{
    char_t* name;
    char_t* mainPath;
    char_t* imgArgs; // Used if the image needs args
    struct boot_entry_s* next;
} boot_entry_s;

boot_entry_s* ParseConfig(void);
int8_t ValidateEntry(boot_entry_s newEntry, boot_entry_s** head);
void AssignValueToEntry(const char_t* key, char_t* value, boot_entry_s* entry);
int8_t ParseLine(boot_entry_s* entry, char_t* token);

boot_entry_s* InitializeEntry(void);
void AppendEntry(boot_entry_s* head, boot_entry_s* entry);
void FreeBootEntries(boot_entry_s* head);
