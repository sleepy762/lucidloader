#include <uefi.h>
#include <filesystem.h>
#include <debug.h>

int main(int argc, char** argv)
{
    // Global status of the bootloader
    efi_status_t status;

    // Disable the Watchdog timer in order to prevent it from restarting the application after 5 minutes
    status = BS->SetWatchdogTimer(0, 0, 0, NULL);
    if (EFI_ERROR(status)) {
        PrintWarning("Failed to disable Watchdog timer, app will restart in 5 minutes.", status);
    }

    // Clear the screen
    ST->ConOut->ClearScreen(ST->ConOut);
    printf("Hello world!\nPress any key to continue...\n");

    // Clear the input buffer
    ST->ConIn->Reset(ST->ConIn, 0);
    efi_input_key_t Key;

    // Wait for a keypress and then print the key
    while ((status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key)) == EFI_NOT_READY);
    printf("Pressed: %c\n", Key.UnicodeChar);

    efi_file_handle_t* rootDir = NULL;

    status = GetRootVolume(IM, &rootDir);
    if (EFI_ERROR(status)) { sleep(3); return status; }
    
    // Open a test file in read mode
    efi_file_handle_t* winBootMgrHandle = NULL;
    uint16_t path[] = u"EFI\\Microsoft\\Boot\\bootmgfw.efi";
    status = rootDir->Open(rootDir, &winBootMgrHandle, path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
    if (EFI_ERROR(status)) {
        PrintError("Couldn't open file 'EFI\\Microsoft\\Boot\\bootmgfw.efi'.", status);
        sleep(3);
        return status;
    }
    DebugPrint("File EFI\\Microsoft\\Boot\\bootmgfw.efi open.\n");


    efi_guid_t infoGuid = EFI_FILE_INFO_GUID;
    efi_file_info_t fileInfo;
    uintn_t infoSize = sizeof(fileInfo);
    status = winBootMgrHandle->GetInfo(winBootMgrHandle, &infoGuid, &infoSize, &fileInfo);
    if (EFI_ERROR(status)) {
        PrintError("Couldn't read the file info.", status);
        sleep(3);
        return status;
    }

    uintn_t winBootMgrSize = fileInfo.FileSize;
    printf("[DEBUG] File size: %ld\n", winBootMgrSize);

    // (char*)malloc(winBootMgrSize + 1)
    char* winBootMgrData = (char*)malloc(winBootMgrSize + 1);
    winBootMgrData[winBootMgrSize] = 0;

    status = winBootMgrHandle->Read(winBootMgrHandle, &winBootMgrSize, winBootMgrData);
    if (EFI_ERROR(status)) {
        PrintError("Couldn't read the file.", status);
        sleep(3);
        return status;
    }

    efi_handle_t imgHandle = 0;
    printf("[DEBUG] image handle %p before LoadImage()\n", imgHandle);
    status = BS->LoadImage(0, IM, LIP->FilePath, winBootMgrData, winBootMgrSize, &imgHandle);
    if (EFI_ERROR(status)) {
        PrintError("Couldn't load image.", status);
        sleep(3);
        return status;
    }
    printf("[DEBUG] image handle %p after LoadImage()\n", imgHandle);

    status = BS->StartImage(imgHandle, NULL, NULL);
    if (EFI_ERROR(status)) {
        PrintError("Couldn't start image.", status);
        sleep(30);
        return status;
    }

    sleep(3);
    
    return 0;
}