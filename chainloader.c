#include "chainloader.h"

void ChainloadImage(wchar_t* path)
{
    efi_device_path_t* devPath = NULL;
    efi_file_handle_t* rootDir = NULL;
    efi_file_handle_t* imgFileHandle = NULL;
    GetFileProtocols(path, &devPath, &rootDir, &imgFileHandle);
    free(path);

    // Get file information for the file size
    efi_guid_t infGuid = EFI_FILE_INFO_GUID;
    efi_file_info_t imgInfo;
    uintn_t size = sizeof(efi_file_info_t);
    efi_status_t status = imgFileHandle->GetInfo(imgFileHandle, &infGuid, &size, (void*)&imgInfo);
    if (EFI_ERROR(status))
        ErrorExit("Failed to get file information.", status);

    // Read the file data into a buffer
    uintn_t imgFileSize = imgInfo.FileSize;
    char* imgData = (char*)malloc(imgFileSize);
    if(!imgData)
        ErrorExit("Out of memory.", EFI_OUT_OF_RESOURCES);

    status = imgFileHandle->Read(imgFileHandle, &imgFileSize, imgData);
    if (EFI_ERROR(status))
        ErrorExit("Failed to read the image file.", status);

    // Load and start the image
    efi_handle_t imgHandle;
    status = BS->LoadImage(0, IM, devPath, imgData, imgFileSize, &imgHandle);
    if (EFI_ERROR(status))
        ErrorExit("Failed to load the image.", status);

    status = BS->StartImage(imgHandle, NULL, NULL);
    if (EFI_ERROR(status))
        ErrorExit("Failed to start the image.", status);
}