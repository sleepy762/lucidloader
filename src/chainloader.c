#include "chainloader.h"

efi_status_t ChainloadImage(char_t* path)
{
    efi_device_path_t* devPath = NULL;
    efi_file_handle_t* rootDir = NULL;
    efi_file_handle_t* imgFileHandle = NULL;
    GetFileProtocols(path, &devPath, &rootDir, &imgFileHandle);

    if (devPath == NULL || rootDir == NULL || imgFileHandle == NULL)
    {
        Log(LL_ERROR, 0, "Failed to get image file protocols for chainloading '%s'.", path);
        return EFI_NOT_FOUND;
    }

    // Get file information for the file size
    efi_file_info_t imgInfo;
    efi_status_t status = GetFileInfo(imgFileHandle, &imgInfo);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to get file information for chainloading '%s'.", path);
        return status;
    }

    // Read the file data into a buffer
    uintn_t imgFileSize = imgInfo.FileSize;
    char_t* imgData = NULL;
    status = ReadFile(imgFileHandle, imgFileSize, &imgData);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to read file '%s' for chainloading.", path);
        return status;
    }

    // Load and start the image
    efi_handle_t imgHandle;
    status = BS->LoadImage(0, IM, devPath, imgData, imgFileSize, &imgHandle);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to load the image for chainloading '%s'.", path);
        return status;
    }
    BS->FreePool(imgData);

    Log(LL_INFO, 0, "Chainloading image '%s'...", path);
    status = BS->StartImage(imgHandle, NULL, NULL);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to start the image '%s' (chainload).", path);
        return status;
    }

    // This line should never be reached
    return 0;
}
