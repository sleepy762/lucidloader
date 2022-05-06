#pragma once
#include <uefi.h>

// Global console size variables
// These variables should be updated ONLY in screen.c functions
extern uintn_t screenRows;
extern uintn_t screenCols;

// A flag that when set to TRUE means that the console size is known and safe to use
extern boolean_t screenModeSet;

boolean_t SetMaxConsoleSize(void);
boolean_t QueryCurrentConsoleSize(void);
void PrepareScreenForRedraw(void);

// Taken from Limine
// https://github.com/limine-bootloader/limine/blob/trunk/common/lib/fb.h
typedef struct framebuffer_t
{
    uint64_t framebuffer_addr;
    uint16_t framebuffer_pitch;
    uint16_t framebuffer_width;
    uint16_t framebuffer_height;
    uint16_t framebuffer_bpp;
    uint8_t  red_mask_size;
    uint8_t  red_mask_shift;
    uint8_t  green_mask_size;
    uint8_t  green_mask_shift;
    uint8_t  blue_mask_size;
    uint8_t  blue_mask_shift;
} framebuffer_t;

framebuffer_t* GetFrameBufferInfo(void);
