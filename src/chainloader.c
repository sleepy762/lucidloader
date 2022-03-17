#include "chainloader.h"

void ChainloadImage(char_t* path, char_t* args)
{
    efi_device_path_t* devPath = NULL;
    efi_file_handle_t* rootDir = NULL;
    efi_file_handle_t* imgFileHandle = NULL;
    efi_status_t status = GetFileProtocols(path, &devPath, &rootDir, &imgFileHandle);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to get image file protocols for chainloading '%s'.", path);
        return;
    }

    // Read the file data into a buffer
    uintn_t imgFileSize = 0;
    char_t* imgData = GetFileContent(path, &imgFileSize);
    if (imgData == NULL)
    {
        Log(LL_ERROR, 0, "Failed to read file '%s' for chainloading: %s", path);
        return;
    }

    // Load the image
    efi_handle_t imgHandle;
    status = BS->LoadImage(0, IM, devPath, imgData, imgFileSize, &imgHandle);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to load the image for chainloading '%s'.", path);
        return;
    }
    free(imgData);

    // Adds arguments to the loaded image, if there are any
    efi_loaded_image_protocol_t* imgProtocol = NULL;
    if (args != NULL)
    {
        efi_guid_t loadedImageGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
        status = BS->HandleProtocol(imgHandle, &loadedImageGuid, (void**)&imgProtocol);
        if (EFI_ERROR(status))
        {
            Log(LL_ERROR, status, "Failed to get loaded image protocol when passing args.");
            return;
        }
        else
        {
            imgProtocol->LoadOptions = StringToWideString(args);
            imgProtocol->LoadOptionsSize = (strlen(args) + 1) * sizeof(wchar_t);
        }
    }

    Log(LL_INFO, 0, "Chainloading image '%s'...", path);
    status = BS->StartImage(imgHandle, NULL, NULL);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to start the image '%s' (chainload).", path);
        return;
    }

    // We shouldn't reach this, but in case the chainload fails we don't want memory leaks
    free(imgProtocol->LoadOptions);
    imgFileHandle->Close(imgFileHandle);
    rootDir->Close(rootDir);
}
