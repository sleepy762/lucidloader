#include "bootmenu.h"

void PrintBootloaderVersion(void)
{
    printf("%s v%s\n\n", BOOTLOADER_NAME_STR, BOOTLOADER_VERSION);
}

void MainMenu(void)
{
    ST->ConOut->ClearScreen(ST->ConOut);
    boot_entry_s* headConfig = ParseConfig();
    
    while (TRUE)
    {
        if (headConfig == NULL)
        {
            FailMenu(BAD_CONFIGURATION_ERR_MSG);
        }
        else
        {
            SuccessMenu(headConfig);
        }
    }
}

void SuccessMenu(boot_entry_s* head)
{
    while (TRUE)
    {
        uint8_t i = 0;
        boot_entry_s* curr = head;

        ST->ConOut->ClearScreen(ST->ConOut);
        
        PrintBootloaderVersion();
        while (curr != NULL)
        {  
            printf("%d. %s, %s\n", ++i, curr->name, curr->mainPath);
            curr = curr->next;
        } 
        
        printf("\nPress 'c' to open the shell\n\n");

        //clear buffer and read key stroke
        ST->ConIn->Reset(ST->ConIn, 0);    
        efi_input_key_t key;

        do
        {
            key = GetInputKey();
        } while (key.UnicodeChar != SHELL_CHAR && 
                ((key.UnicodeChar > i + CHAR_INT) || (key.UnicodeChar < '1')));
        
        if(key.UnicodeChar == SHELL_CHAR)
        {
            StartShell();
            continue; // Return to the beginning
        }

        curr = GetCurrOS(key.UnicodeChar - CHAR_INT, head);

        // Printing info before booting
        ST->ConOut->ClearScreen(ST->ConOut);
        printf("Booting `%s`...\n", curr->name);
        printf("- path: `%s`\n", curr->mainPath);
        printf("- args: `%s`\n\n", curr->imgArgs);

        ChainloadImage(curr->mainPath, curr->imgArgs);

        // If booting failed we break the loop in order to show the fail menu
        break;
    }
    FailMenu(FAILED_BOOT_ERR_MSG);
}

void FailMenu(const char_t* errorMsg)
{
    boolean_t returnToMainMenu = FALSE;
    while (!returnToMainMenu)
    {
        ST->ConOut->ClearScreen(ST->ConOut);

        PrintBootloaderVersion();
        printf("%s\n\n", errorMsg);
        printf("1) Open shell    (fix/change configuration file)\n");
        printf("2) Show log\n");
        printf("3) Shutdown\n");
        printf("4) Restart\n");
        printf("5) Return to main menu\n");
        
        //clear buffer and read key stroke
        ST->ConIn->Reset(ST->ConIn, 0);    
        efi_input_key_t key;

        do
        {
            key = GetInputKey();
        } while ((key.UnicodeChar < '1') || (key.UnicodeChar > '5'));
        //check if key is valid af
        
        switch(key.UnicodeChar)
        {
            case '1':
                StartShell();
                break;
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

boot_entry_s * GetCurrOS(uint8_t numOfPartition, boot_entry_s * head)
{
    uint8_t  i = 0;
    boot_entry_s * curr = head;

    for(i = 1; i < numOfPartition; i++)
    {
        curr = curr->next;
    }
    
    return curr;
}
