#include <filesystem.h>
#include <debug.h>

// Opens the root directory of the EFI System Partition volume
// IM          - IN Image handle
// imgProtocol - OUT Image protocol
// rootDir     - OUT Root volume
efi_status_t GetRootVolume(efi_handle_t IM, efi_file_handle_t** rootDir)
{
    efi_status_t status;
    
    // Open the file system protocol, with the device handle, to access files
    efi_handle_t device = LIP->DeviceHandle;
    efi_simple_file_system_protocol_t* fsProtocol = NULL;
    efi_guid_t fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    status = BS->OpenProtocol(device, &fsGuid, (void**)&fsProtocol, IM, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (EFI_ERROR(status)) {
        PrintError("Couldn't open the file system protocol.", status);
        return status;
    }
    DebugPrint("File system protocol open.\n");

    // Open the root directory of the EFI system partition, initializes (*rootDir)
    status = fsProtocol->OpenVolume(fsProtocol, rootDir);
    if (EFI_ERROR(status)) {
        PrintError("Couldn't open the root volume", status);
        return status;
    }
    DebugPrint("Root directory volume opened.\n");
    
    return status;
}