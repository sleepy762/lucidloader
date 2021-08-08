#include <uefi.h>
#include <filesystem.h>

int main(int argc, char** argv)
{
    // Global status of the bootloader
    efi_status_t status;

    // Disable the Watchdog timer in order to prevent it from restarting the application after 5 minutes
    status = ST->BootServices->SetWatchdogTimer(0, 0, 0, NULL);
    if (EFI_ERROR(status)) {
        printf("[WARNING] Failed to disable Watchdog timer, app will restart in 5 minutes. (%ld)\n", status);
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


    efi_loaded_image_protocol_t* imgProtocol = NULL;
    efi_file_handle_t* rootDir = NULL;

    status = GetRootVolume(IM, &imgProtocol, &rootDir);
    if (EFI_ERROR(status)) { sleep(3); return status; }

    printf("[DEBUG] Image Protocol open at 0x%p\n", imgProtocol);
    printf("[DEBUG] Root directory open at 0x%p\n", rootDir);

    /*
    // Open a test file in read mode
    efi_file_handle_t* testFile = NULL;
    uint16_t path[] = u"EFI\\test\\test1.efi";
    status = rootDir->Open(rootDir, &testFile, path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
    if (EFI_ERROR(status)) {
        printf("[ERROR] Couldn't open the file. (%ld)\n", status);
        sleep(3);
        return status;
    }
    printf("[DEBUG] File open\n");

    // Get file information
    efi_guid_t infGuid = EFI_FILE_INFO_GUID; 
    efi_file_info_t fileInfo;
    uintn_t size = sizeof(fileInfo);
    status = testFile->GetInfo(testFile, &infGuid, &size, (void*)&fileInfo);
    if (EFI_ERROR(status)) {
        printf("[ERROR] Couldn't get file info. (%ld)\n", status);
        sleep(3);
        return status;
    }

    printf("[DEBUG] Filename: ");
    // Use OutputString() in order to print wchar_t strings
    ST->ConOut->OutputString(ST->ConOut, fileInfo.FileName);

    // Use formatting %ld to print uint64_t values
    printf(", size: %ld bytes.\n", fileInfo.FileSize);
    
    status = rootDir->Close(testFile);
    if (EFI_ERROR(status)) {
        printf("[ERROR] Couldn't close the file. (%ld)\n", status);
        sleep(3);
        return status;
    }
    */

    sleep(3);
    
    return 0;
}