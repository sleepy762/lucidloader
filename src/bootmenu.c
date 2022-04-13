#include "bootmenu.h"
#include "shell.h"
#include "config.h"
#include "chainloader.h"
#include "logger.h"
#include "version.h"
#include "bootutils.h"
#include "editor.h"
#include "shellutils.h"
#include "screen.h"

#define F5_KEY_SCANCODE (0x0F) // Used to refresh the menu (reparse config)

#define SHELL_CHAR  ('c')
#define INFO_CHAR   ('i')

#define BAD_CONFIGURATION_ERR_MSG ("An error has occurred while parsing the config file.")
#define FAILED_BOOT_ERR_MSG ("An error has occurred during the booting process.")

/* Menu functions */
static void BootMenu(boot_entry_array_s* entryArr);
static void FailMenu(const char_t* errorMsg);

/* Wrappers */
static inline void BootHighlightedEntry(boot_entry_array_s* entryArr);
static inline void PrintHighlightedEntryInfo(boot_entry_array_s* entryArr);

/* Etc */
static void InitBootMenuConfig(void);
static void BootEntry(boot_entry_s* selectedEntry);
static void PrintEntryInfo(boot_entry_s* selectedEntry);
static void ScrollEntryList(void);

/* Output */
static void PrintBootMenu(boot_entry_array_s* entryArr);
static void PrintMenuEntries(boot_entry_array_s* entryArr);
static inline void PrintInstructions(void);
static void PrintTimeout(void);

boot_menu_cfg_s bmcfg;


void PrintBootloaderVersion(void)
{
    printf("%s v%s\n\n", LUCIDLOADER_NAME_STR, LUCIDLOADER_VERSION);
}

void StartBootloader(void)
{
    InitBootMenuConfig();
    while (TRUE)
    {
        ST->ConOut->ClearScreen(ST->ConOut);
        ST->ConOut->SetAttribute(ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
        ST->ConOut->EnableCursor(ST->ConOut, FALSE);
        PrintBootloaderVersion();
        printf("Parsing config...\n");

        // The config parsing is in this loop because we want the menu to update in case the user
        // decided to update the config through the bootloader shell
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
    // This variable defines the amount of entries that can be shown on screen at once
    // We subtract because there are rows that we have reserved for other printing
    bmcfg.maxEntriesOnScreen = screenRows - 10;

    bmcfg.selectedEntryIndex = 0;
    bmcfg.entryOffset = 0;
    bmcfg.timeoutSeconds = 10; // Default value
    bmcfg.timeoutCancelled = FALSE;
    bmcfg.bootImmediately = FALSE;
}

static void PrintMenuEntries(boot_entry_array_s* entryArr)
{
    int32_t index = bmcfg.entryOffset; // The index at which the printed entries begin

    // Print how many hidden entries are at the top of the list
    if (index > 0)
    {
        printf(" . . . %d more", index);
        PadRow();
    }
    else
    {
        PrintEmptyLine();
    }

    for (int32_t i = 0; i < bmcfg.maxEntriesOnScreen; i++)
    {
        // Prevent going out of bounds
        if (index >= entryArr->numOfEntries)
        {
            break;
        }

        int32_t entryNum = index + 1;
        char_t* entryName = entryArr->entries[index].name;
        if (index == bmcfg.selectedEntryIndex) // Highlight the selected entry
        {
            ST->ConOut->SetAttribute(ST->ConOut, EFI_TEXT_ATTR(EFI_BLACK, EFI_LIGHTGRAY));
            printf(" %d) %s ", entryNum, entryName);
            ST->ConOut->SetAttribute(ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
        }
        else // Print normally
        {
            printf(" %d) %s ", entryNum, entryName);
        }
        PadRow();

        index++;
    }

    // Print how many hidden entries are at the bottom of the list
    if (index < entryArr->numOfEntries)
    {
        printf(" . . . %d more", entryArr->numOfEntries - index);
        PadRow();
    }
    else
    {
        PrintEmptyLine();
    }
}

static inline void PrintInstructions(void)
{
    printf("\nUse the up and down arrow keys to select which entry is highlighted.\n"
        "Press enter to boot the selected entry, 'c' to open the shell\n"
        "'i' to get info about a highlighted entry, or F5 to refresh the menu.\n");
}

static void PrintTimeout(void)
{
    ST->ConOut->SetAttribute(ST->ConOut, EFI_TEXT_ATTR(EFI_WHITE, EFI_BLACK));
    printf("The highlighted selection will be booted automatically in %d seconds.", bmcfg.timeoutSeconds);
    ST->ConOut->SetAttribute(ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
    PadRow();
}

static void PrintBootMenu(boot_entry_array_s* entryArr)
{
    // Setting cursor position instead of clearing screen in order to prevent flicker
    ST->ConOut->SetCursorPosition(ST->ConOut, 0, 0);
    PrintBootloaderVersion();

    PrintMenuEntries(entryArr);
    
    PrintInstructions();

    if (!bmcfg.timeoutCancelled)
    {
        PrintTimeout();
    }
    else
    {
        PrintEmptyLine();
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
    ST->ConOut->ClearScreen(ST->ConOut);
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
    ST->ConOut->ClearScreen(ST->ConOut);
}

void ShowLogFile(void)
{
    ST->ConOut->ClearScreen(ST->ConOut);

    PrintLogFile();

    printf("\nPress any key to return...");
    GetInputKey();
}
