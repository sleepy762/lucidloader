#include <chainloader.h>

void ChainloadImage(wchar_t* path)
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
    efi_device_path_t* devPath = NULL;
    efi_guid_t devGuid = EFI_DEVICE_PATH_PROTOCOL_GUID;

    efi_file_handle_t* rootDir = NULL;
    efi_file_handle_t* imgFileHandle = NULL;
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

        status = BS->HandleProtocol(handle, &devGuid, (void**)&devPath);
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
        status = sfsProt->OpenVolume(sfsProt, &rootDir);
        if (EFI_ERROR(status))
            continue;

        // Get a handle to the image file
        status = rootDir->Open(rootDir, &imgFileHandle, path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
        if (EFI_ERROR(status))
            PrintDebug("Checking another partition for the file...\n");
    }
    if(imgFileHandle == NULL) 
        ErrorExit("Failed to find the image on the machine.", EFI_NOT_FOUND);
    free(path);

    // Get file information for the file size
    efi_guid_t infGuid = EFI_FILE_INFO_GUID;
    efi_file_info_t imgInfo;
    uintn_t size = sizeof(efi_file_info_t);
    status = imgFileHandle->GetInfo(imgFileHandle, &infGuid, &size, (void*)&imgInfo);
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