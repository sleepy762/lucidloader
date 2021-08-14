#include <uefi.h>
#include "debug.h"

wchar_t* StringToWideString(const char* str);
void GetFileProtocols(wchar_t* path, efi_device_path_t** devPath, efi_file_handle_t** rootDir, efi_file_handle_t** imgFileHandle);