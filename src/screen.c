#include "screen.h"
#include "logger.h"
#include "bootutils.h"

uintn_t screenRows = DEFAULT_CONSOLE_ROWS;
uintn_t screenCols = DEFAULT_CONSOLE_COLUMNS;

boolean_t SetMaxConsoleSize(void)
{
    int32_t maxMode = ST->ConOut->Mode->MaxMode - 1;

    uintn_t maxModeCols = 0;
    uintn_t maxModeRows = 0;
    efi_status_t status = ST->ConOut->QueryMode(ST->ConOut, maxMode, &maxModeCols, &maxModeRows);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to query max mode console size (Mode %d).", maxMode);
        return FALSE;
    }

    // Set the global variables
    screenRows = maxModeRows;
    screenCols = maxModeCols;

    if (ST->ConOut->Mode->Mode == maxMode)
    {
        Log(LL_INFO, 0, "Max mode console size of %dx%d is already set (Mode %d).", 
            maxModeCols, maxModeRows, maxMode);
        return TRUE;
    }

    status = ST->ConOut->SetMode(ST->ConOut, maxMode);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to set console size %dx%d (Mode %d).", 
            maxModeCols, maxModeRows, maxMode);
        return FALSE;
    }

    Log(LL_INFO, 0, "Console size set to %dx%d (Mode %d).", 
        maxModeCols, maxModeRows, maxMode);

    return TRUE;
}
