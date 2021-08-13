#include <uefi.h>
#include <debug.h>

int main(int argc, char** argv)
{
    // Global status of the bootloader
    efi_status_t status;

    // Disable the Watchdog timer in order to prevent it from restarting the application after 5 minutes
    status = BS->SetWatchdogTimer(0, 0, 0, NULL);
    if (EFI_ERROR(status))
        PrintWarning("Failed to disable Watchdog timer, app will restart in 5 minutes.", status);

    // Clear the screen
    ST->ConOut->ClearScreen(ST->ConOut);
    printf("Hello world!\nPress any key to continue...\n");

    // Clear the input buffer
    ST->ConIn->Reset(ST->ConIn, 0);
    efi_input_key_t Key;

    // Wait for a keypress and then print the key
    while ((status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key)) == EFI_NOT_READY);
    printf("Pressed: %c\n", Key.UnicodeChar);
    
    // Get all the simple file system protocol handles
    efi_guid_t sfsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    uintn_t bufSize = 0;
    efi_handle_t* handles= NULL;
    status = BS->LocateHandle(ByProtocol, &sfsGuid, NULL, &bufSize, handles);
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
    }

    // Open the root volume
    efi_file_handle_t* rootDir = NULL;
    status = sfsProt->OpenVolume(sfsProt, &rootDir);
    if (EFI_ERROR(status))
        ErrorExit("Failed to open root volume.", status);

    // Get a handle to the windows boot manager file
    efi_file_handle_t* winBootMgrHandle = NULL;
    wchar_t path[] = u"EFI\\apps\\bootmgfw.efi";
    status = rootDir->Open(rootDir, &winBootMgrHandle, path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
    if (EFI_ERROR(status))
        ErrorExit("Failed to open the windows boot manager.", status);

    // Get file information for the file size
    efi_guid_t infGuid = EFI_FILE_INFO_GUID;
    efi_file_info_t winBootMgrInfo;
    uintn_t size = sizeof(efi_file_info_t);
    status = winBootMgrHandle->GetInfo(winBootMgrHandle, &infGuid, &size, (void*)&winBootMgrInfo);
    if (EFI_ERROR(status))
        ErrorExit("Failed to get file information.", status);

    // Read the file data into a buffer
    uintn_t winBootMgrFileSize = winBootMgrInfo.FileSize;
    char* winBootMgrData = (char*)malloc(winBootMgrFileSize);
    if(!winBootMgrData)
        ErrorExit("Out of memory.", EFI_OUT_OF_RESOURCES);

    status = winBootMgrHandle->Read(winBootMgrHandle, &winBootMgrFileSize, winBootMgrData);
    if (EFI_ERROR(status))
        ErrorExit("Failed to read the windows boot manager file.", status);

    // Load and start the windows boot manager
    efi_handle_t winBootMgrImgHandle;
    status = BS->LoadImage(0, IM, devPath, winBootMgrData, winBootMgrFileSize, &winBootMgrImgHandle);
    if (EFI_ERROR(status))
        ErrorExit("Failed to load the windows boot manager image.", status);

    status = BS->StartImage(winBootMgrImgHandle, NULL, NULL);
    if (EFI_ERROR(status))
        ErrorExit("Failed to start the windows boot manager.", status);
    
    // This should never be reached
    return 0;
}