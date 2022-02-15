#pragma once
#include <uefi.h>
#include "logger.h"

// Taken from the UEFI Specification v2.9
#define EFI_OS_INDICATIONS_BOOT_TO_FW_UI (0x0000000000000001)

#define FALSE ((boolean_t)0)
#define TRUE ((boolean_t)1)

#define DEFAULT_WATCHDOG_TIMEOUT (300)

wchar_t* StringToWideString(char_t* str);

efi_status_t GetFileProtocols(char_t* path, efi_device_path_t** devPath, efi_file_handle_t** rootDir, efi_file_handle_t** fileHandle);
efi_status_t GetFileInfo(efi_file_handle_t* fileHandle, efi_file_info_t* fileInfo);
efi_status_t ReadFile(efi_file_handle_t* fileHandle, uintn_t fileSize, char_t** buffer);

char_t* GetFileContent(char_t* path);
uint64_t GetFileSize(FILE* file);

efi_status_t RebootDevice(boolean_t rebootToFirmware);
efi_status_t ShutdownDevice(void);

void DisableWatchdogTimer(void);
void EnableWatchdogTimer(uintn_t seconds);
