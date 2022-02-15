#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "shellerr.h"
#include "logger.h"
#include "efiextendedinput.h"

#define EDITOR_EXIT_KEY ('q')

#define UP_ARROW_SCANCODE       (0x01)
#define DOWN_ARROW_SCANCODE     (0x02)
#define RIGHT_ARROW_SCANCODE    (0x03)
#define LEFT_ARROW_SCANCODE     (0x04)

#define PAGEUP_KEY_SCANCODE     (0x09)
#define PAGEDOWN_KEY_SCANCODE   (0x0A)

#define HOME_KEY_SCANCODE       (0x05)
#define END_KEY_SCANCODE        (0x06)

#define DELETE_KEY_SCANCODE     (0x08)

typedef struct text_row_t
{
    uint32_t size;
    char_t* chars;
} text_row_t;

typedef struct editor_config_t
{
    // Size of the screen in rows and columns
    uintn_t screenRows;
    uintn_t screenCols;

    // The current cursor location
    uintn_t cx;
    uintn_t cy;
    
    // Stores the text of the opened file
    uint32_t numRows;
    text_row_t* row;
} editor_config_t;

typedef struct buffer_t
{
    char_t* b;
    uint32_t len;
} buffer_t;
#define BUF_INIT {NULL, 0}

boolean_t IsKeyPressedWithLCtrl(efi_key_data_t keyData, char_t key);

int8_t StartEditor(char_t* filename);
efi_key_data_t GetInputKeyData(efi_simple_text_input_ex_protocol_t* ConInEx);

void AppendToBuffer(buffer_t* buf, const char_t* str, uint32_t len);
void PrintBuffer(buffer_t* buf);
void FreeBuffer(buffer_t* buf);
