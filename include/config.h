#pragma once
#include <uefi.h>
#include "debug.h"
#include "bootutils.h"

// Temporary path
static wchar_t* cfgPath = u"EFI\\apps\\config.cfg";
static char* cfgLineDelimiter = "\n";
static char* cfgEntryDelimiter = "\n\n";
static const char keyValueDelimiter = '=';

typedef enum {Chainload=1, Linux} boot_type_t;

typedef struct
{
    char* initrdPath;
    char* kernelArgs;
} linux_values_s;

// General struct which applies to any type of booting
// The mainPath variable depends on the boot type -
// When chainloading, it holds the path to the image to be chainloaded
// With Linux, it holds the path to the kernel to be loaded
typedef struct boot_entry_s
{
    char* name;
    boot_type_t type;
    char* mainPath;
    linux_values_s linuxValues;
    struct boot_entry_s* next;
} boot_entry_s;

boot_entry_s* ParseConfig(void);
void ValidateEntry(boot_entry_s newEntry, boot_entry_s** head);
void AssignValueToEntry(const char* key, char* value, boot_entry_s* entry);
void ParseLine(boot_entry_s* entry, char* token);

boot_entry_s* InitializeEntry(void);
void AppendEntry(boot_entry_s* head, boot_entry_s* entry);
