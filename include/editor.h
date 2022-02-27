#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "shellerr.h"
#include "logger.h"

/* Some editor configuration macros */

// Control keys
#define EDITOR_EXIT_CTRL_KEY     (0x11)
#define EDITOR_SAVE_CTRL_KEY     (0x13)
#define EDITOR_SEARCH_CTRL_KEY   (0x06)

// How many times the user has to press the quit key to exit if the file has been modified
#define EDITOR_QUIT_TIMES (3)

#define EDITOR_STATUS_MSG_ARR_SIZE (80)
#define EDITOR_WELCOME_MSG_ARR_SIZE (80)

#define EDITOR_INITIAL_STATUS_MSG ("HELP: CTRL-Q = quit | CTRL-S = save | CTRL-F = search")
#define EDITOR_STATUS_MSG_TIMEOUT (5)

// How many spaces to replace TABs with
#define EDITOR_TAB_SIZE (4)

// Used in the input processing function
#define UP_ARROW_SCANCODE       (0x01)
#define DOWN_ARROW_SCANCODE     (0x02)
#define RIGHT_ARROW_SCANCODE    (0x03)
#define LEFT_ARROW_SCANCODE     (0x04)

#define PAGEUP_KEY_SCANCODE     (0x09)
#define PAGEDOWN_KEY_SCANCODE   (0x0A)

#define HOME_KEY_SCANCODE       (0x05)
#define END_KEY_SCANCODE        (0x06)

#define DELETE_KEY_SCANCODE     (0x08)

#define ESCAPE_KEY_SCANCODE     (0x17)

// Struct specific to the editor
typedef struct text_row_t
{
    int32_t size;
    char_t* chars;

    int32_t rsize;
    char_t* render;
} text_row_t;

// Struct specific to the editor
typedef struct editor_config_t
{
    // Size of the screen in rows and columns
    intn_t screenRows;
    intn_t screenCols;

    // The current cursor location
    intn_t cx;
    intn_t cy;
    intn_t rx; // Location in the render buffer
    
    // Stores the text of the opened file
    intn_t numRows;
    text_row_t* row;
    intn_t rowOffset;
    intn_t colOffset;

    // Misc
    char_t* fullFilePath; // The full path of the file
    char_t* filename; // Rendered filename string
    char_t statusmsg[EDITOR_STATUS_MSG_ARR_SIZE];
    time_t statusmsgTime;
    boolean_t dirty; // Modified without saving
} editor_config_t;

// Generic buffer struct
typedef struct buffer_t
{
    char_t* b;
    int32_t len;
} buffer_t;
#define BUF_INIT {NULL, 0} // Used for initializing the buffer

int8_t StartEditor(char_t* filename);

void AppendToBuffer(buffer_t* buf, const char_t* str, uint32_t len);
void PrintBuffer(buffer_t* buf);
void FreeBuffer(buffer_t* buf);
