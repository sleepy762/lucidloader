#include "protocols/efilaunch.h"
#include <uefi.h>
#include "logger.h"
#include "bootutils.h"

void StartEFIImage(char* path, char* args)
{
    // Device handle must be passed to the loaded image protocol because of
    // the way we call LoadImage()
    efi_handle_t devHandle = GetFileDeviceHandle(path);
    if (devHandle == NULL)
    {
        Log(LL_ERROR, 0, "Failed to get file device handle for '%s'.", path);
        return;
    }

    efi_device_path_t* devPath = NULL;
    efi_guid_t devPathGuid = EFI_DEVICE_PATH_PROTOCOL_GUID;
    efi_status_t status = BS->HandleProtocol(devHandle, &devPathGuid, (void**)&devPath);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to handle device path.");
        return;
    }

    // Read the file data into a buffer
    uintn_t imgFileSize = 0;
    char_t* imgData = GetFileContent(path, &imgFileSize);
    if (imgData == NULL)
    {
        Log(LL_ERROR, 0, "Failed to read file '%s'.", path);
        return;
    }

    // Load the image
    efi_handle_t imgHandle;
    status = BS->LoadImage(FALSE, IM, devPath, imgData, imgFileSize, &imgHandle);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to load EFI image '%s'.", path);
        goto cleanup;
    }

    efi_guid_t loadedImageGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    efi_loaded_image_protocol_t* imgProtocol = NULL;
    status = BS->HandleProtocol(imgHandle, &loadedImageGuid, (void**)&imgProtocol);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to get loaded image protocol when passing args.");
        goto cleanup;
    }
    else
    {
        // Adds arguments to the loaded image, if there are any
        if (args != NULL)
        {
            imgProtocol->LoadOptions = StringToWideString(args);
            imgProtocol->LoadOptionsSize = (strlen(args) + 1) * sizeof(wchar_t);
        }
        // Calling LoadImage() with the image data isn't going to set the device handle
        // so we have to do it. This also fixes an EFI stub issue with linux kernels on
        // some firmwares, where kernel efi_mains would fail with the error
        // 'failed to handle fs_proto' 
        imgProtocol->DeviceHandle = devHandle;
    }

    Log(LL_INFO, 0, "Starting EFI image '%s'...", path);
    status = BS->StartImage(imgHandle, NULL, NULL);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to start the image '%s'.", path);
    }

cleanup:
    // We shouldn't reach this, but in case starting fails we don't want memory leaks
    if (args != NULL)
    {
        free(imgProtocol->LoadOptions);
    }
    free(imgData);
}
