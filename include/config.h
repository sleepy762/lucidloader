#pragma once
#include <uefi.h>

typedef struct boot_entry_s
{
    char_t* name; // Name in the menu
    char_t* mainPath; // Holds a path to the file to load (or directory with a linux kernel)
    char_t* imgArgs; // Used if the image needs args

    // The purpose is to have the bootloader automatically detect the version string of
    // the (linux) kernel and substitute it wherever needed in the args, in order to
    // avoid having to edit the config file with every kernel version update
    boolean_t isDirectoryToKernel;
} boot_entry_s;
#define BOOT_ENTRY_INIT { NULL, NULL, NULL, 0 }

typedef struct boot_entry_array_s
{
    boot_entry_s* entries;
    int32_t numOfEntries;
} boot_entry_array_s;
#define BOOT_ENTRY_ARR_INIT { NULL, 0 }

boot_entry_array_s ParseConfig(void);
void ParseKeyValuePair(char_t* token, const char_t delimiter, char_t** key, char_t** value);
void FreeConfigEntries(boot_entry_array_s* entryArr);
