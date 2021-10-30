#include <uefi.h>
#include "logger.h"
#include "chainloader.h"
#include "config.h"
#include "shell.h"

int main(int argc, char** argv)
{
    // Global status of the bootloader
    efi_status_t status;

    if(!InitLogger())
    {
        printf("Failed to initialize logger. Logging disabled.\n");
    }

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
    (void)entries; // Suppress the warning
    ChainloadImage(StringToWideString("EFI\\apps\\bootmgfw.efi"));
    
    // This should never be reached
    return 0;
}
