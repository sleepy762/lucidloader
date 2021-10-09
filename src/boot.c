#include <uefi.h>
#include "debug.h"
#include "chainloader.h"
#include "config.h"
#include "shell.h"

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
    printf("Hello world!\nPress `c` to open the shell or press any other key to continue...\n");

    // Clear the input buffer
    ST->ConIn->Reset(ST->ConIn, 0);
    efi_input_key_t Key;

    // Wait for a keypress and then print the key
    while ((status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key)) == EFI_NOT_READY);
    printf("Pressed: %d %d\n", Key.UnicodeChar, Key.ScanCode);

    if (Key.UnicodeChar == 'c') StartShell();

    boot_entry_s* entries = ParseConfig();
    ChainloadImage(StringToWideString("EFI\\apps\\bootmgfw.efi"));
    
    // This should never be reached
    return 0;
}
