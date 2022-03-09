#include "bootmenu.h"

static void BootMenu(boot_entry_array_s* entryArr);
static void FailMenu(const char_t* errorMsg);

void PrintBootloaderVersion(void)
{
    printf("%s v%s\n\n", BOOTLOADER_NAME_STR, BOOTLOADER_VERSION);
}

void MainMenu(void)
{
    ST->ConOut->ClearScreen(ST->ConOut);
    ST->ConOut->SetAttribute(ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
    
    while (TRUE)
    {
        // The config parsing is in this loop because we want the menu to update in case the user
        // decided to update the config through the bootloader shell
        boot_entry_array_s bootEntries = ParseConfig();

        if (bootEntries.numOfEntries == 0)
        {
            FailMenu(BAD_CONFIGURATION_ERR_MSG);
        }
        else
        {
            BootMenu(&bootEntries);
        }

        FreeConfigEntries(&bootEntries);
    }
}

static void BootMenu(boot_entry_array_s* entryArr)
{
    ST->ConOut->ClearScreen(ST->ConOut);
    PrintBootloaderVersion();
        
    for (int i = 0; i < entryArr->numOfEntries; i++)
    {
        printf("%d) %s, %s\n", i + 1, entryArr->entries[i].name, entryArr->entries[i].mainPath);
    }
        
    printf("\nUse the up and down arrow keys to select which entry is highlighted.\n"
           "Press enter to boot the selected entry.\n"
           "Press 'c' to open the shell.\n\n");

    // Clear buffer and read key stroke
    ST->ConIn->Reset(ST->ConIn, 0);
    efi_input_key_t key;
    do
    {
        key = GetInputKey();
    } while (key.UnicodeChar != SHELL_CHAR &&
            ((key.UnicodeChar > entryArr->numOfEntries + '0') || (key.UnicodeChar < '1')));

    if(key.UnicodeChar == SHELL_CHAR)
    {
        StartShell();
        return;
    }

    boot_entry_s selectedEntry = entryArr->entries[key.UnicodeChar - '1'];

    // Printing info before booting
    ST->ConOut->ClearScreen(ST->ConOut);
    printf("Booting `%s`...\n"
            "- path: `%s`\n"
            "- args: `%s`\n",
            selectedEntry.name, selectedEntry.mainPath, selectedEntry.imgArgs);

    ChainloadImage(selectedEntry.mainPath, selectedEntry.imgArgs);

    // If booting failed we will end up here
    FailMenu(FAILED_BOOT_ERR_MSG);
}

static void FailMenu(const char_t* errorMsg)
{
    boolean_t returnToMainMenu = FALSE;
    while (!returnToMainMenu)
    {
        ST->ConOut->ClearScreen(ST->ConOut);

        PrintBootloaderVersion();
        printf("%s\n\n", errorMsg);
        printf("1) Open shell    (fix/change configuration file)\n"
               "2) Show log\n"
               "3) Shutdown\n"
               "4) Restart\n"
               "5) Return to main menu\n");
        
        //clear buffer and read key stroke
        ST->ConIn->Reset(ST->ConIn, 0);    
        efi_input_key_t key;

        // check if key is valid
        do
        {
            key = GetInputKey();
        } while ((key.UnicodeChar < '1') || (key.UnicodeChar > '5'));

        switch(key.UnicodeChar)
        {
            case '1':
                StartShell();
                return; // Return in order to parse the config again
            case '2':
                ShowLogFile();
                break;
            case '3':
                ShutdownDevice();
                break;
            case '4':
                RebootDevice(FALSE);
                break;
            case '5':
                returnToMainMenu = TRUE;
                break;
            default:
                // nothing
                break;
        }
    }
}

void ShowLogFile(void)
{
    ST->ConOut->ClearScreen(ST->ConOut);

    uint8_t res = PrintFileContent(LOG_PATH);
    if (res != CMD_SUCCESS)
    {
        printf("Failed to open log file!\n");
    }

    printf("\nPress any key to continue...");
    GetInputKey();
}
