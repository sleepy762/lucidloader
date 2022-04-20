#include "screen.h"
#include "logger.h"
#include "bootutils.h"

uintn_t screenRows = DEFAULT_CONSOLE_ROWS;
uintn_t screenCols = DEFAULT_CONSOLE_COLUMNS;
boolean_t screenModeSet = FALSE;

boolean_t SetMaxConsoleSize(void)
{
    int32_t maxMode = ST->ConOut->Mode->MaxMode - 1;

    // Query the console size of maxmode
    uintn_t maxModeCols = 0;
    uintn_t maxModeRows = 0;
    efi_status_t status = ST->ConOut->QueryMode(ST->ConOut, maxMode, &maxModeCols, &maxModeRows);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to query max mode console size (Mode %d).", maxMode);
        return FALSE;
    }

    // Check if the console size is already at the maximum
    if (ST->ConOut->Mode->Mode == maxMode)
    {
        Log(LL_INFO, 0, "Max mode console size of %dx%d is already set (Mode %d).", 
            maxModeCols, maxModeRows, maxMode);
        goto success;
    }

    // Set the max console size
    status = ST->ConOut->SetMode(ST->ConOut, maxMode);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to set console size %dx%d (Mode %d).", 
            maxModeCols, maxModeRows, maxMode);
        return FALSE;
    }
    Log(LL_INFO, 0, "Console size set to %dx%d (Mode %d).", 
        maxModeCols, maxModeRows, maxMode);

success:
    screenRows = maxModeRows;
    screenCols = maxModeCols;
    screenModeSet = TRUE;
    return TRUE;
}

// This should be used if setting the max mode failed
boolean_t QueryCurrentConsoleSize(void)
{
    efi_status_t status = ST->ConOut->QueryMode(ST->ConOut, 
        ST->ConOut->Mode->Mode, &screenCols, &screenRows);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to query current console size (Mode %d).", ST->ConOut->Mode->Mode);
        screenModeSet = FALSE;
    }
    else
    {
        screenModeSet = TRUE;
    }
    return screenModeSet;
}

// Used wherever the boot manager is about to redraw the screen.
// If the screen size was properly set, then the boot manager will try to change
// the cursor position to the top left corner and redraw without clearing the screen
// to prevent flickering. However if it wasn't set correctly, then the boot manager has
// to clear the screen because it doesn't know the console size.
void PrepareScreenForRedraw(void)
{
    if (screenModeSet)
    {
        ST->ConOut->SetCursorPosition(ST->ConOut, 0, 0);
    }
    else
    {
        ST->ConOut->ClearScreen(ST->ConOut);
    }
}
