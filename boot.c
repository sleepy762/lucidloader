#include <uefi.h>
#include "debug.h"
#include "chainloader.h"
#include "config.h"

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

    boot_entry_s* entries = ParseConfig();
    printf("---successs----\n");
    boot_entry_s* copy = entries;
    while(copy != NULL)
    {
        printf("name:%s\n", copy->name);
        printf("type:%d\n", copy->type);
        printf("path:%s\n", copy->mainPath);
        if(copy->type == Linux)
        {
            printf("initrd:%s\n", copy->linuxValues.initrdPath);
            printf("args:%s\n", copy->linuxValues.kernelArgs);
        }
        copy = copy->next;
        printf("\n");
    }
    sleep(5);
    ChainloadImage(StringToWideString("EFI\\apps\\bootmgfw.efi"));
    
    // This should never be reached
    return 0;
}