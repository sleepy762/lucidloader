#include "chainloader.h"

int ChainloadImage(wchar_t* path)
{
    efi_device_path_t* devPath = NULL;
    efi_file_handle_t* rootDir = NULL;
    efi_file_handle_t* imgFileHandle = NULL;
    GetFileProtocols(path, &devPath, &rootDir, &imgFileHandle);
    BS->FreePool(path);

    if (devPath == NULL || rootDir == NULL || imgFileHandle == NULL)
    {
        Log(LL_ERROR, 0, "Failed to get image file protocols for chainloading.");
        return 1;
    }

    // Get file information for the file size
    efi_file_info_t imgInfo;
    efi_status_t status = GetFileInfo(imgFileHandle, &imgInfo);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to get file information for chainloading.");
        return 1;
    }

    // Read the file data into a buffer
    uintn_t imgFileSize = imgInfo.FileSize;
    char* imgData = NULL;
    status = ReadFile(imgFileHandle, imgFileSize, &imgData);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to read file for chainloading.");
        return 1;
    }

    // Load and start the image
    efi_handle_t imgHandle;
    status = BS->LoadImage(0, IM, devPath, imgData, imgFileSize, &imgHandle);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to load the image for chainloading.");
        return 1;
    }
    BS->FreePool(imgData);

    Log(LL_INFO, 0, "Chainloading image...");
    status = BS->StartImage(imgHandle, NULL, NULL);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to start the image (chainload).");
        return 1;
    }

    return 0;
}
