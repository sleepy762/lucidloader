#include "utils.h"

wchar_t* StringToWideString(const char* str)
{
    const size_t size = strlen(str);
    wchar_t* wpath = (wchar_t*)malloc(size + 1);
    wpath[size] = 0;
    mbstowcs(wpath, str, size);
    return wpath;
}

void GetFileProtocols(wchar_t* path, efi_device_path_t** devPath, efi_file_handle_t** rootDir, efi_file_handle_t** imgFileHandle)
{
    // Get all the simple file system protocol handles
    efi_guid_t sfsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    uintn_t bufSize = 0;
    efi_handle_t* handles= NULL;
    efi_status_t status = BS->LocateHandle(ByProtocol, &sfsGuid, NULL, &bufSize, handles);
    if (status != EFI_BUFFER_TOO_SMALL)
        ErrorExit("Initial location of the simple file system protocol handles failed.", status);

    handles = (efi_handle_t*)malloc(bufSize);
    if(!handles)
        ErrorExit("Out of memory.", EFI_OUT_OF_RESOURCES);

    status = BS->LocateHandle(ByProtocol, &sfsGuid, NULL, &bufSize, handles);
    if (EFI_ERROR(status))
        ErrorExit("Unable to locate the simple file system protocol handles.", status);

    // Find the right protocols
    uintn_t numHandles = bufSize / sizeof(efi_handle_t);
    efi_simple_file_system_protocol_t* sfsProt = NULL;
    efi_guid_t devGuid = EFI_DEVICE_PATH_PROTOCOL_GUID;

    for (int i = 0; i < numHandles; i++)
    {
        efi_handle_t handle = handles[i];
        status = BS->HandleProtocol(handle, &sfsGuid, (void**)&sfsProt);
        if (EFI_ERROR(status)) 
        {
            if (i + 1 == numHandles) 
                ErrorExit("Unable to obtain the simple file system protocol.", status);
            continue;
        }

        status = BS->HandleProtocol(handle, &devGuid, (void**)devPath);
        if (EFI_ERROR(status))
        {
            if (i + 1 == numHandles)
                ErrorExit("Unable to obtain the device path protocol.", status);
            continue;
        }

        // This is a way to check that we are accessing the right FAT volume
        // It is not perfect since if a file with the same name exists on 2 different FAT volumes
        // The bootloader will load the first instance

        // Open the root volume
        status = sfsProt->OpenVolume(sfsProt, rootDir);
        if (EFI_ERROR(status))
            continue;

        // Get a handle to the image file
        status = (*rootDir)->Open((*rootDir), imgFileHandle, path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
        if (EFI_ERROR(status))
            PrintDebug("Checking another partition for the file...\n");
    }
    if((*imgFileHandle) == NULL) 
        ErrorExit("Failed to find the image on the machine.", EFI_NOT_FOUND);
}