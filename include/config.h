#pragma once
#include <uefi.h>

typedef enum boot_protocol_t
{
    BP_EFI_LAUNCH,
    BP_LINUX
} boot_protocol_t;

typedef struct kernel_scan_info_s
{
    char_t* kernelDirectory;
    char_t* kernelVersionString;
} kernel_scan_info_s;

typedef struct boot_entry_s
{
    char_t* name; // Name in the menu
    char_t* imgToLoad; // Holds a path to the file to load
    char_t* imgArgs; // Used if the image needs args
    boot_protocol_t bootProtocol;
    const char_t* bootProtocolStr;

    // The purpose is to have the boot manager automatically detect the version string of
    // the (linux, for now) kernel and substitute it wherever needed in the args, in order to
    // avoid having to edit the config file with every kernel version update
    boolean_t isDirectoryToKernel;
    kernel_scan_info_s* kernelScanInfo;
} boot_entry_s;

typedef struct boot_entry_array_s
{
    boot_entry_s* entries;
    int32_t numOfEntries;
} boot_entry_array_s;

boot_entry_array_s ParseConfig(void);
boolean_t ParseKeyValuePair(char_t* token, const char_t delimiter, char_t** key, char_t** value);
void FreeConfigEntries(boot_entry_array_s* entryArr);
