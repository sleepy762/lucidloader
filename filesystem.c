#include <filesystem.h>

// Opens the root directory of the EFI System Partition volume
// IM          - IN Image handle
// imgProtocol - OUT Image protocol
// rootDir     - OUT Root volume
efi_status_t GetRootVolume(efi_handle_t IM, efi_loaded_image_protocol_t** imgProtocol, efi_file_handle_t** rootDir)
{
    efi_status_t status;

    // Open image protocol to get the device handle, initializes (*imgProtocol)
    efi_guid_t imgGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    status = BS->OpenProtocol(IM, &imgGuid, (void**)imgProtocol, IM, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (EFI_ERROR(status)) {
        printf("[ERROR] Couldn't open the image protocol. (%ld)\n", status);
        return status;
    }
    printf("[DEBUG] Image protocol open.\n");
    
    // Open the file system protocol, with the device handle, to access files
    efi_handle_t device = (*imgProtocol)->DeviceHandle;
    efi_simple_file_system_protocol_t* fsProtocol = NULL;
    efi_guid_t fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    status = BS->OpenProtocol(device, &fsGuid, (void**)&fsProtocol, IM, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (EFI_ERROR(status)) {
        printf("[ERROR] Couldn't open the file system protocol. (%ld)\n", status);
        return status;
    }
    printf("[DEBUG] File system protocol open.\n");

    // Open the root directory of the EFI system partition, initializes (*rootDir)
    status = fsProtocol->OpenVolume(fsProtocol, rootDir);
    if (EFI_ERROR(status)) {
        printf("[ERROR] Couldn't open the root volume. (%ld)\n", status);
        return status;
    }
    printf("[DEBUG] Root directory volume opened.\n");
    
    return status;
}