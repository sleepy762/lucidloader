#include "chainloader.h"

void ChainloadImage(wchar_t* path)
{
    efi_device_path_t* devPath = NULL;
    efi_file_handle_t* rootDir = NULL;
    efi_file_handle_t* imgFileHandle = NULL;
    GetFileProtocols(path, &devPath, &rootDir, &imgFileHandle);
    BS->FreePool(path);

    // Get file information for the file size
    efi_file_info_t imgInfo;
    efi_status_t status = GetFileInfo(imgFileHandle, &imgInfo);
    if (EFI_ERROR(status))
        ErrorExit("Failed to get file information.", status);

    // Read the file data into a buffer
    uintn_t imgFileSize = imgInfo.FileSize;
    char* imgData = NULL;
    status = ReadFile(imgFileHandle, imgFileSize, &imgData);

    // Load and start the image
    efi_handle_t imgHandle;
    status = BS->LoadImage(0, IM, devPath, imgData, imgFileSize, &imgHandle);
    if (EFI_ERROR(status))
        ErrorExit("Failed to load the image.", status);
    BS->FreePool(imgData);

    status = BS->StartImage(imgHandle, NULL, NULL);
    if (EFI_ERROR(status))
        ErrorExit("Failed to start the image.", status);
}
