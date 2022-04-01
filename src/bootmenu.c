#include "bootmenu.h"
#include "shell.h"
#include "config.h"
#include "chainloader.h"
#include "logger.h"
#include "version.h"
#include "bootutils.h"
#include "editor.h"
#include "shellutils.h"

#define F5_KEY_SCANCODE (0x0F) // Used to refresh the menu (reparse config)

#define SHELL_CHAR  ('c')
#define INFO_CHAR   ('i')

#define BAD_CONFIGURATION_ERR_MSG ("An error has occurred while parsing the config file.")
#define FAILED_BOOT_ERR_MSG ("An error has occurred during the booting process.")

static void BootMenu(boot_entry_array_s* entryArr);
static void FailMenu(const char_t* errorMsg);

static inline void BootHighlightedEntry(boot_entry_array_s* entryArr);
static inline void PrintHighlightedEntryInfo(boot_entry_array_s* entryArr);

static void InitBootMenuConfig(void);
static void BootEntry(boot_entry_s* selectedEntry);
static void PrintEntryInfo(boot_entry_s* selectedEntry);
static void PrintBootMenu(boot_entry_array_s* entryArr);
static void ScrollEntryList(void);

boot_menu_cfg_s bmcfg;


void PrintBootloaderVersion(void)
{
    printf("%s v%s\n\n", EZBOOT_NAME_STR, EZBOOT_VERSION);
}

void StartBootloader(void)
{
    InitBootMenuConfig();
    while (TRUE)
    {
        ST->ConOut->ClearScreen(ST->ConOut);
        ST->ConOut->SetAttribute(ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
        PrintBootloaderVersion();
        printf("Parsing config...\n");

        // The config parsing is in this loop because we want the menu to update in case the user
        // decided to update the config through the bootloader shell
        // TODO: only reparse if config was modified
        boot_entry_array_s bootEntries = ParseConfig();

        ST->ConIn->Reset(ST->ConIn, 0);
        if (bootEntries.numOfEntries == 0)
        {
            FailMenu(BAD_CONFIGURATION_ERR_MSG);
        }
        else
        {
            BootMenu(&bootEntries);
        }

        FreeConfigEntries(&bootEntries);
        bmcfg.selectedEntryIndex = 0;
        bmcfg.entryOffset = 0;
    }
}

static void InitBootMenuConfig(void)
{
    uintn_t rows = DEFAULT_CONSOLE_ROWS;
    efi_status_t status = ST->ConOut->QueryMode(ST->ConOut, ST->ConOut->Mode->Mode, NULL, &rows);
    if (EFI_ERROR(status))
    {
        Log(LL_WARNING, status, "Failed to query the console size in the boot menu.");
        rows = DEFAULT_CONSOLE_ROWS;
    }

    // This variable defines the amount of entries that can be shown on screen at once
    // We subtract because there are rows that we have reserved for other printing
    bmcfg.maxEntriesOnScreen = rows - 10;

    bmcfg.selectedEntryIndex = 0;
    bmcfg.entryOffset = 0;
    bmcfg.timeoutSeconds = 10; // Default value
    bmcfg.timeoutCancelled = FALSE;
    bmcfg.bootImmediately = FALSE;
}

static void PrintBootMenu(boot_entry_array_s* entryArr)
{
    ST->ConOut->ClearScreen(ST->ConOut);
    PrintBootloaderVersion();

    int32_t index = bmcfg.entryOffset; // The index at which the printed entries begin

    // Print how many hidden entries are at the top of the list
    if (index > 0)
    {
        printf(" . . . %d more\n", index);
    }
    else
    {
        putchar('\n');
    }

    for (int32_t i = 0; i < bmcfg.maxEntriesOnScreen; i++)
    {   
        // Prevent going out of bounds
        if (index >= entryArr->numOfEntries)
        {
            break;
        }

        if (index == bmcfg.selectedEntryIndex) // Highlight the selected entry
        {
            ST->ConOut->SetAttribute(ST->ConOut, EFI_TEXT_ATTR(EFI_BLACK, EFI_LIGHTGRAY));
            printf(" %d) %s \n", index + 1, entryArr->entries[index].name);
            ST->ConOut->SetAttribute(ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
        }
        else // Print normally
        {
            printf(" %d) %s \n", index + 1, entryArr->entries[index].name);
        }
        index++;
    }

    // Print how many hidden entries are at the bottom of the list
    if (index < entryArr->numOfEntries)
    {
        printf(" . . . %d more\n", entryArr->numOfEntries - index);
    }
    else
    {
        putchar('\n');
    }
    
    printf("\nUse the up and down arrow keys to select which entry is highlighted.\n"
           "Press enter to boot the selected entry, 'c' to open the shell\n"
           "'i' to get info about a highlighted entry, or F5 to refresh the menu.\n");

    // Show the timeout
    if (!bmcfg.timeoutCancelled)
    {
        ST->ConOut->SetAttribute(ST->ConOut, EFI_TEXT_ATTR(EFI_WHITE, EFI_BLACK));
        printf("The highlighted selection will be booted automatically in %d seconds.\n", bmcfg.timeoutSeconds);
        ST->ConOut->SetAttribute(ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
    }
}

static void BootMenu(boot_entry_array_s* entryArr)
{
    while (TRUE)
    {
        PrintBootMenu(entryArr);

        if (!bmcfg.timeoutCancelled)
        {
            if (bmcfg.bootImmediately)
            {
                BootHighlightedEntry(entryArr);
                return;
            }

            int32_t timerStatus = WaitForInput(1000);
            if (timerStatus == INPUT_TIMER_TIMEOUT)
            {
                bmcfg.timeoutSeconds--;
                // Boot the selected entry if the timer ends
                if (bmcfg.timeoutSeconds == 0)
                {
                    BootHighlightedEntry(entryArr);
                    return;
                }
                continue;
            }
            else if (timerStatus == INPUT_TIMER_KEY)
            {
                // Cancel the timer if a key was pressed
                bmcfg.timeoutCancelled = TRUE;
            }
        }
        efi_input_key_t key = GetInputKey();

        switch (key.ScanCode)
        {
            case UP_ARROW_SCANCODE:
                if (bmcfg.selectedEntryIndex != 0)
                {
                    bmcfg.selectedEntryIndex--;
                    ScrollEntryList();
                }
                break;
            case DOWN_ARROW_SCANCODE:
                if (bmcfg.selectedEntryIndex + 1 < entryArr->numOfEntries)
                {
                    bmcfg.selectedEntryIndex++;
                    ScrollEntryList();
                }
                break;
            case F5_KEY_SCANCODE:
                // Return to reparse the config
                return;

            default:
                switch (key.UnicodeChar)
                {
                    case CHAR_CARRIAGE_RETURN:
                        BootHighlightedEntry(entryArr);
                        break;

                    case SHELL_CHAR:
                        StartShell();
                        break;

                    case INFO_CHAR:
                        PrintHighlightedEntryInfo(entryArr);
                        break;
                    
                    default:
                        // Nothing
                        break;
                }
        }
    }
}

static void ScrollEntryList(void)
{
    if (bmcfg.selectedEntryIndex < bmcfg.entryOffset) // Scroll up
    {
        bmcfg.entryOffset = bmcfg.selectedEntryIndex;
    }
    if (bmcfg.selectedEntryIndex >= bmcfg.entryOffset + bmcfg.maxEntriesOnScreen) // Scroll down
    {
        bmcfg.entryOffset = bmcfg.selectedEntryIndex - bmcfg.maxEntriesOnScreen + 1;
    }
}

static void PrintEntryInfo(boot_entry_s* selectedEntry)
{
    ST->ConOut->ClearScreen(ST->ConOut);
    PrintBootloaderVersion();

    printf("Entry %d\n\n", bmcfg.selectedEntryIndex + 1);
    printf("Name: %s\n"
           "Path: %s\n"
           "Args: %s\n",
           selectedEntry->name, selectedEntry->imgToLoad, selectedEntry->imgArgs);
    
    if (selectedEntry->isDirectoryToKernel)
    {
        printf("\nKernel directory: %s\n", selectedEntry->kernelScanInfo->kernelDirectory);
        printf("Kernel version string: %s\n", selectedEntry->kernelScanInfo->kernelVersionString);
    }

    printf("\nPress any key to return...");
    GetInputKey();
}

static inline void PrintHighlightedEntryInfo(boot_entry_array_s* entryArr)
{
    PrintEntryInfo(&entryArr->entries[bmcfg.selectedEntryIndex]);
}

static inline void BootHighlightedEntry(boot_entry_array_s* entryArr)
{
    BootEntry(&entryArr->entries[bmcfg.selectedEntryIndex]);
    // If booting fails we will end up here
    FailMenu(FAILED_BOOT_ERR_MSG);
}

static void BootEntry(boot_entry_s* selectedEntry)
{
    // Printing info before booting
    ST->ConOut->ClearScreen(ST->ConOut);
    printf("Booting `%s`...\n"
            "- path: `%s`\n"
            "- args: `%s`\n\n",
            selectedEntry->name, selectedEntry->imgToLoad, selectedEntry->imgArgs);

    ChainloadImage(selectedEntry->imgToLoad, selectedEntry->imgArgs);
}

static void FailMenu(const char_t* errorMsg)
{
    bmcfg.timeoutCancelled = TRUE;
    boolean_t returnToMainMenu = FALSE;

    while (!returnToMainMenu)
    {
        ST->ConOut->ClearScreen(ST->ConOut);

        PrintBootloaderVersion();
        printf("%s\n\n", errorMsg);
        printf("1) Open shell\n"
               "2) Show log\n"
               "3) Shutdown\n"
               "4) Restart\n"
               "5) Return to main menu\n\n"
               "Press a number on your keyboard to select an option.\n");
        
        //clear buffer and read key stroke
        ST->ConIn->Reset(ST->ConIn, 0);    
        efi_input_key_t key = {0};

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

    PrintLogFile();

    printf("\nPress any key to return...");
    GetInputKey();
}
