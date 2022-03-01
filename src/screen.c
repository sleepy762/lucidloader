#include "screen.h"

boolean_t SetMaxConsoleSize(void)
{
    int32_t maxMode = ST->ConOut->Mode->MaxMode - 1;

    uintn_t maxModeCols;
    uintn_t maxModeRows;
    efi_status_t status = ST->ConOut->QueryMode(ST->ConOut, maxMode, &maxModeCols, &maxModeRows);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to query max mode console size (Mode %d).", maxMode);
        return FALSE;
    }

    status = ST->ConOut->SetMode(ST->ConOut, maxMode);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to set console size %dx%d (Mode %d).", maxModeCols, maxModeRows, maxMode);
        return FALSE;
    }
    Log(LL_INFO, 0, "Console size set to %dx%d (Mode %d).", maxModeCols, maxModeRows, maxMode);

    return TRUE;
}
