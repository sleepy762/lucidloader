#include "screen.h"
#include "logger.h"
#include "bootutils.h"

static void linear_mask_to_mask_shift(uint8_t *mask, uint8_t *shift, uint32_t linear_mask);
static uint16_t linear_masks_to_bpp(uint32_t red_mask, uint32_t green_mask,
                                    uint32_t blue_mask, uint32_t alpha_mask);

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

// Taken from Limine
// https://github.com/limine-bootloader/limine/blob/trunk/common/drivers/gop.c
static uint16_t linear_masks_to_bpp(uint32_t red_mask, uint32_t green_mask,
                                    uint32_t blue_mask, uint32_t alpha_mask) 
{
    uint32_t compound_mask = red_mask | green_mask | blue_mask | alpha_mask;
    uint16_t ret = 32;
    while ((compound_mask & (1 << 31)) == 0) 
    {
        ret--;
        compound_mask <<= 1;
    }
    return ret;
}

static void linear_mask_to_mask_shift(uint8_t *mask, uint8_t *shift, uint32_t linear_mask) 
{
    *shift = 0;
    while ((linear_mask & 1) == 0) 
    {
        (*shift)++;
        linear_mask >>= 1;
    }
    *mask = 0;
    while ((linear_mask & 1) == 1) 
    {
        (*mask)++;
        linear_mask >>= 1;
    }
}

framebuffer_t* GetFrameBufferInfo(void)
{
    efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    efi_gop_t* gop = NULL;

    efi_status_t status = BS->LocateProtocol(&gopGuid, NULL, (void**)&gop);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to locate the GOP protocol.");
        return NULL;
    }

    efi_gop_mode_info_t* gopModeInfo = NULL;
    uintn_t gopModeInfoSize = 0;

    status = gop->QueryMode(gop, gop->Mode->Mode, &gopModeInfoSize, &gopModeInfo);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to query current GOP mode.");
        return NULL;
    }

    framebuffer_t* fb = malloc(sizeof(framebuffer_t));
    if (fb == NULL)
    {
        Log(LL_ERROR, 0, "Failed to allocate memory for the frame buffer.");
        return NULL;
    }

    switch (gopModeInfo->PixelFormat)
    {
    case PixelBlueGreenRedReserved8BitPerColor:
        fb->framebuffer_bpp = 32;
        fb->red_mask_size = 8;
        fb->red_mask_shift = 16;
        fb->green_mask_size = 8;
        fb->green_mask_shift = 8;
        fb->blue_mask_size = 8;
        fb->blue_mask_shift = 0;
        break;

    case PixelRedGreenBlueReserved8BitPerColor:
        fb->framebuffer_bpp = 32;
        fb->red_mask_size = 8;
        fb->red_mask_shift = 0;
        fb->green_mask_size = 8;
        fb->green_mask_shift = 8;
        fb->blue_mask_size = 8;
        fb->blue_mask_shift = 16;
        break;
    
    case PixelBitMask:
        fb->framebuffer_bpp = linear_masks_to_bpp(
                                gopModeInfo->PixelInformation.RedMask,
                                gopModeInfo->PixelInformation.GreenMask,
                                gopModeInfo->PixelInformation.RedMask,
                                gopModeInfo->PixelInformation.ReservedMask);
        linear_mask_to_mask_shift(&fb->red_mask_size,
                                  &fb->red_mask_shift,
                                  gopModeInfo->PixelInformation.RedMask);
        linear_mask_to_mask_shift(&fb->green_mask_size,
                                  &fb->green_mask_shift,
                                  gopModeInfo->PixelInformation.GreenMask);
        linear_mask_to_mask_shift(&fb->blue_mask_size,
                                  &fb->blue_mask_shift,
                                  gopModeInfo->PixelInformation.BlueMask);
        break;

    default:
        Log(LL_ERROR, 0, "Invalid GOP PixelFormat. (%d)", gopModeInfo->PixelFormat);
        return NULL;
    }
    fb->framebuffer_addr = gop->Mode->FrameBufferBase;
    fb->framebuffer_pitch = gop->Mode->Information->PixelsPerScanLine * (fb->framebuffer_bpp / 8);
    fb->framebuffer_width = gop->Mode->Information->HorizontalResolution;
    fb->framebuffer_height = gop->Mode->Information->VerticalResolution;
    // clear fb

    return fb;
}
