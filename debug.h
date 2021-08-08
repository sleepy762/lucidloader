#include <uefi.h>

// Comment out this line in order to disable debug info.
#define DEBUG

void DebugPrint(const char* string);
void PrintError(const char* moreInfo, efi_status_t status);
void PrintWarning(const char* moreInfo, efi_status_t status);
const char* GetErrorInfo(efi_status_t status);