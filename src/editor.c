// Editor implementation inspired by snaptoken's kilo tutorial
// https://github.com/snaptoken/kilo-src

// The original source code was written by Salvatore Sanfilippo (aka antirez)
// and released under the BSD 2-clause license (see below). The code was modified
// to work in UEFI.
/*
Copyright (c) 2016, Salvatore Sanfilippo <antirez at gmail dot com>

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "editor.h"
#include "shellutils.h"
#include "shellerr.h"
#include "bootutils.h"
#include "logger.h"
#include "screen.h"

#define EDITOR_STATUS_MSG_ARR_SIZE (80)
#define EDITOR_WELCOME_MSG_ARR_SIZE (80)

/* Some editor configuration macros */

// How many times the user has to press the quit key to exit if the file has been modified
#define EDITOR_QUIT_TIMES (3)

// Status bar
#define EDITOR_INITIAL_STATUS_MSG ("HELP: ESC = quit | F1 = save | F2 = find")
#define EDITOR_STATUS_MSG_TIMEOUT (5)

// How many spaces to replace TABs with
#define EDITOR_TAB_SIZE (4)

// Keys with special functionality
#define EDITOR_EXIT_KEY         (ESCAPE_KEY_SCANCODE)
#define EDITOR_SAVE_KEY         (F1_KEY_SCANCODE)
#define EDITOR_SEARCH_KEY       (F2_KEY_SCANCODE)

// Struct specific to the editor
typedef struct text_row_s
{
    int32_t size;
    char_t* chars;

    int32_t rsize;
    char_t* render;
} text_row_s;

// Struct specific to the editor
typedef struct editor_config_s
{
    // Size of the screen in rows and columns
    intn_t editorRows;
    intn_t editorCols;

    // The current cursor location
    intn_t cx;
    intn_t cy;
    intn_t rx; // Location in the render buffer
    
    // Stores the text of the opened file
    intn_t numRows;
    text_row_s* row;
    intn_t rowOffset;
    intn_t colOffset;

    // Misc
    char_t* fullFilePath; // The full path of the file
    char_t* filename; // Rendered filename string
    char_t statusmsg[EDITOR_STATUS_MSG_ARR_SIZE];
    time_t statusmsgTime;
    boolean_t dirty; // Modified without saving
} editor_config_s;

#define BUF_INIT {NULL, 0} // Used for initializing the buffer

/*** Static declarations ***/
/* Row operations */
static void EditorInsertRow(int32_t at, char_t* str, size_t len);
static void EditorUpdateRow(text_row_s* row);
static intn_t EditorRowCxToRx(text_row_s* row, intn_t cx);
static intn_t EditorRowRxToCx(text_row_s* row, intn_t rx);
static void EditorRowInsertChar(text_row_s* row, int32_t at, char_t c);
static void EditorRowDeleteChar(text_row_s* row, int32_t at);
static void EditorFreeRow(text_row_s* row);
static void EditorDeleteRow(int32_t at);
static void EditorRowAppendString(text_row_s* row, char_t* str, size_t len);

/* Editor operations */
static void EditorInsertChar(char_t c);
static void EditorDeleteChar(void);
static void EditorInsertNewline(void);

/* File I/O */
static boolean_t EditorOpenFile(char_t* filename);
static char_t* EditorRowsToString(int32_t* buflen);
static void EditorSave(void);

/* Search */
static void EditorFind(void);
static void EditorFindCallback(char_t* query, efi_input_key_t key);

/* Output */
static void AppendEditorWelcomeMessage(buffer_s* buf);
static void EditorRefreshScreen(void);
static void EditorDrawRows(buffer_s* buf);
static void EditorScroll(void);
static void EditorDrawStatusBar(void);
static void EditorSetStatusMessage(const char_t* fmt, ...);
static void EditorDrawMessageBar(void);

/* Input */
static boolean_t ProcessEditorInput(void);
static void EditorMoveCursor(uint16_t scancode);
static char_t* EditorPrompt(const char_t* prompt, void (*callback)(char_t*, efi_input_key_t));

/* Init and finish */
static void InitEditorConfig(void);
static void FreeEditorMemory(void);

static editor_config_s cfg;


int8_t StartEditor(char_t* filename)
{
    InitEditorConfig();
    
    if (filename != NULL)
    {
        if (!EditorOpenFile(filename))
        {
            return CMD_OUT_OF_MEMORY;
        }
    }

    EditorSetStatusMessage(EDITOR_INITIAL_STATUS_MSG);

    ST->ConIn->Reset(ST->ConIn, 0);
    // Keep reading and processing input until the editor is closed
    do
    {
        EditorRefreshScreen();
    } while (ProcessEditorInput());
    
    FreeEditorMemory();
    ST->ConOut->ClearScreen(ST->ConOut);
    return 0;
}

// Free all the dynamically allocated memory
static void FreeEditorMemory(void)
{
    if (cfg.fullFilePath != NULL)
    {
        free(cfg.fullFilePath);
    }
    if (cfg.row != NULL)
    {
        for (intn_t i = 0; i < cfg.numRows; i++)
        {
            free(cfg.row[i].chars);
            free(cfg.row[i].render);
        }
        free(cfg.row);
    }
}

static void InitEditorConfig(void)
{
    const int32_t reserveRows = 2;
    if (screenModeSet)
    {
        cfg.editorCols = screenCols;
        cfg.editorRows = screenRows - reserveRows; // Making room for the status messages
    }
    else
    {
        cfg.editorCols = DEFAULT_CONSOLE_COLUMNS;
        cfg.editorRows = DEFAULT_CONSOLE_ROWS - reserveRows;
    }

    cfg.cx = 0;
    cfg.cy = 0;
    cfg.rx = 0;
    cfg.numRows = 0;
    cfg.rowOffset = 0;
    cfg.colOffset = 0;
    cfg.row = NULL;
    cfg.fullFilePath = NULL;
    cfg.filename = NULL;
    cfg.statusmsg[0] = CHAR_NULL;
    cfg.statusmsgTime = 0;
    cfg.dirty = FALSE;
}

// Return value of TRUE means success, FALSE means failure
static boolean_t EditorOpenFile(char_t* filename)
{
    free(cfg.fullFilePath);
    cfg.fullFilePath = strdup(filename);

    // Stores only the filename without the full path, we add 1 to pass the delimiter
    cfg.filename = strrchr(cfg.fullFilePath, '\\') + 1;

    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
    {
        // Don't create the file yet if it doesn't exist, but let the user know it's "modified"
        // When the user saves, the file will be created with all the content
        cfg.dirty = TRUE;
        return TRUE;
    }

    char_t* origDataPtr = GetFileContent(filename, NULL);
    if (origDataPtr == NULL)
    {
        return FALSE;
    }

    char_t* fileData = origDataPtr;
    char_t* token = NULL;

    // Read the file line by line
    while ((token = strtok_r_with_empty_str(fileData, "\n", &fileData)) != NULL)
    {
        size_t linelen = strlen(token);
        while (linelen > 0 && (token[linelen - 1] == '\n' || token[linelen - 1] == '\r'))
        {
            linelen--;
        }
        EditorInsertRow(cfg.numRows, token, linelen);
    }
    cfg.dirty = FALSE;

    free(origDataPtr);
    fclose(fp);
    return TRUE;
}

// Return FALSE when we want to stop processing input
static boolean_t ProcessEditorInput(void)
{
    static int32_t quitTimes = EDITOR_QUIT_TIMES;

    efi_input_key_t key = GetInputKey();

    // EVERY OTHER KEY
    char_t unicodechar = key.UnicodeChar;
    uint16_t scancode = key.ScanCode;
    // In the outer switch we check SPECIAL KEYS (by checking the scancode)
    switch (scancode)
    {
        // SPECIAL FUNCTION KEYS
        case EDITOR_EXIT_KEY:
            // Don't exit from modified files immediately
            if (cfg.dirty && quitTimes > 0)
            {
                EditorSetStatusMessage("WARNING!!! File has unsaved changes. "
                    "Press ESC %d more times to quit.", quitTimes);
                quitTimes--;
                return TRUE;
            }
            else
            {
                return FALSE;
            }
            break;

        case EDITOR_SAVE_KEY:
            EditorSave();
            break;

        case EDITOR_SEARCH_KEY:
            EditorFind();
            break;

        // Scroll a page up/down
        case PAGEUP_KEY_SCANCODE:
        case PAGEDOWN_KEY_SCANCODE:
            if (scancode == PAGEUP_KEY_SCANCODE)
            {
                cfg.cy = cfg.rowOffset;
            }
            else if (scancode == PAGEDOWN_KEY_SCANCODE)
            {                
                cfg.cy = cfg.rowOffset + cfg.editorRows - 1;
                if (cfg.cy > cfg.numRows)
                {
                    cfg.cy = cfg.numRows;
                }
            }

            intn_t times = cfg.editorRows;
            while (times--)
            {
                EditorMoveCursor(scancode == PAGEUP_KEY_SCANCODE ? 
                    UP_ARROW_SCANCODE : DOWN_ARROW_SCANCODE);
            }
            break;

        // Move the cursor to the end or the beginning of the row
        case HOME_KEY_SCANCODE:
            cfg.cx = 0;
            break;
        case END_KEY_SCANCODE:
            if (cfg.cy < cfg.numRows)
            {
                cfg.cx = cfg.row[cfg.cy].size;
            }
            break;

        case DELETE_KEY_SCANCODE:
            EditorDeleteChar();
            break;

        // Move the cursor one column/row
        case UP_ARROW_SCANCODE:
        case DOWN_ARROW_SCANCODE:
        case RIGHT_ARROW_SCANCODE:
        case LEFT_ARROW_SCANCODE:
            EditorMoveCursor(scancode);
            break;

        default:
            // In this inner switch we check the UNICODE CHARS
            switch (unicodechar)
            {
                case CHAR_CARRIAGE_RETURN:
                    EditorInsertNewline();
                    break;

                case CHAR_BACKSPACE:
                    EditorDeleteChar();
                    break;

                default:
                    EditorInsertChar(unicodechar);
                    break;
            }
            break;
    }
    quitTimes = EDITOR_QUIT_TIMES;
    return TRUE;
}

static void AppendEditorWelcomeMessage(buffer_s* buf)
{
    // Add our welcome message into a buffer
    char_t welcome[EDITOR_WELCOME_MSG_ARR_SIZE];
    int32_t welcomelen = snprintf(welcome, sizeof(welcome), "LucidLoader Editor");
    if (welcomelen > cfg.editorCols)
    {
        welcomelen = cfg.editorCols;
    }

    // Add padding to center the welcome message
    intn_t padding = (cfg.editorCols - welcomelen) / 2;
    if (padding)
    {
        AppendToBuffer(buf, "~", 1);
        padding--;
    }
    while (padding--)
    {
        AppendToBuffer(buf, " ", 1);
    }

    // Append the entire message to the buffer
    AppendToBuffer(buf, welcome, welcomelen);
}

static void EditorRefreshScreen(void)
{
    buffer_s buf = BUF_INIT;

    EditorScroll();
    EditorDrawRows(&buf);

    ST->ConOut->EnableCursor(ST->ConOut, FALSE);
    PrepareScreenForRedraw();

    PrintBuffer(&buf);
    EditorDrawStatusBar();
    EditorDrawMessageBar();

    // The offset must be subtracted from the cursor offset, otherwise the value refers to the position
    // of the cursor within the text file and not the position on screen
    ST->ConOut->SetCursorPosition(ST->ConOut, cfg.rx - cfg.colOffset, cfg.cy - cfg.rowOffset);
    ST->ConOut->EnableCursor(ST->ConOut, TRUE);

    FreeBuffer(&buf);
}

static void EditorDrawRows(buffer_s* buf)
{
    for (intn_t y = 0; y < cfg.editorRows; y++)
    {
        // Add offset to know what row of the file the user is currently at
        intn_t fileRow = y + cfg.rowOffset;
        if (fileRow >= cfg.numRows)
        {
            // Write a welcome message if no file was loaded
            if (cfg.numRows == 0 && y == cfg.editorRows / 3)
            {
                AppendEditorWelcomeMessage(buf);
            }
            else // This character is used to show a line after EOF
            {
                AppendToBuffer(buf, "~", 1);
            }
        }
        else // Append the data of the file lines
        {
            int32_t len = cfg.row[fileRow].rsize - cfg.colOffset;
            if (len < 0)
            {
                len = 0;
            }
            if (len >= cfg.editorCols)
            {
                len = cfg.editorCols - 1;
            }
            AppendToBuffer(buf, &cfg.row[fileRow].render[cfg.colOffset], len);
        }
        AppendToBuffer(buf, "\n", 1);
    }
}

static void EditorMoveCursor(uint16_t scancode)
{
    // Used to limit scrolling to the right
    text_row_s* row = (cfg.cy >= cfg.numRows) ? NULL : &cfg.row[cfg.cy];

    // Move the cursor while performing bounds checking
    switch (scancode)
    {
        case UP_ARROW_SCANCODE:
            if (cfg.cy != 0)
            {
                cfg.cy--;
            }
            break;
        case DOWN_ARROW_SCANCODE:
            if (cfg.cy < cfg.numRows)
            {
                cfg.cy++;
            }
            break;
        case RIGHT_ARROW_SCANCODE:
            if (row != NULL && cfg.cx < row->size)
            {
                cfg.cx++;
            }
            else if (row != NULL && cfg.cx == row->size) // Handle moving right at the end of a line
            {
                cfg.cy++;
                cfg.cx = 0;
            }
            break;
        case LEFT_ARROW_SCANCODE:
            if (cfg.cx != 0)
            {
                cfg.cx--;
            }
            else if (cfg.cy > 0) // Handle moving left at the start of a line
            {
                cfg.cy--;
                cfg.cx = cfg.row[cfg.cy].size;
            }
            break;
    }

    // Used to correct the cursor's X position when moving down from a long line to a shorter one
    row = (cfg.cy >= cfg.numRows) ? NULL : &cfg.row[cfg.cy];
    int32_t rowLen = row != NULL ? row->size : 0;
    if (cfg.cx > rowLen)
    {
        cfg.cx = rowLen;
    }
}

static char_t* EditorPrompt(const char_t* prompt, void (*callback)(char_t*, efi_input_key_t))
{
    size_t bufsize = 64;
    char_t* buf = malloc(bufsize);

    size_t buflen = 0;
    buf[0] = CHAR_NULL;

    while (TRUE)
    {
        EditorSetStatusMessage(prompt, buf);
        EditorRefreshScreen();

        efi_input_key_t key = GetInputKey();
        char_t unicodechar = key.UnicodeChar;
        uint16_t scancode = key.ScanCode;

        // Append character to buffer (any printable character)
        if (IsPrintableChar(unicodechar))
        {
            if (buflen == bufsize - 1)
            {
                bufsize *= 2;
                buf = realloc(buf, bufsize);
            }
            buf[buflen++] = unicodechar;
            buf[buflen] = CHAR_NULL;
        }
        // Remove the last character from the buffer (backspace key)
        else if (scancode == DELETE_KEY_SCANCODE || unicodechar == CHAR_BACKSPACE)
        {
            if (buflen != 0)
            {
                buf[--buflen] = CHAR_NULL;
            }
        }
        // Stop reading input and return the buffer (enter key)
        else if (unicodechar == CHAR_CARRIAGE_RETURN)
        {   
            if (buflen != 0)
            {
                EditorSetStatusMessage("");
                if (callback != NULL)
                {
                    callback(buf, key);
                }
                return buf;
            }
        }
        // Stop reading input and return NULL (escape key)
        else if (scancode == ESCAPE_KEY_SCANCODE)
        {
            EditorSetStatusMessage("");
            if (callback != NULL)
            {
                callback(buf, key);
            }
            free(buf);
            return NULL;
        }

        if (callback != NULL)
        {
            callback(buf, key);
        }
    }
}

static void EditorScroll(void)
{
    cfg.rx = 0;
    if (cfg.cy < cfg.numRows)
    {
        cfg.rx = EditorRowCxToRx(&cfg.row[cfg.cy], cfg.cx);
    }

    if (cfg.cy < cfg.rowOffset) // Scroll up
    {
        cfg.rowOffset = cfg.cy;
    }
    if (cfg.cy >= cfg.rowOffset + cfg.editorRows) // Scroll down
    {
        cfg.rowOffset = cfg.cy - cfg.editorRows + 1;
    }

    if (cfg.rx < cfg.colOffset) // Scroll left
    {
        cfg.colOffset = cfg.rx;
    }
    if (cfg.rx >= cfg.colOffset + cfg.editorCols) // Scroll right
    {
        cfg.colOffset = cfg.rx - cfg.editorCols + 1;
    }
}

static void EditorDrawStatusBar(void)
{
    // Sets the colors of the status bar
    ST->ConOut->SetAttribute(ST->ConOut, EFI_TEXT_ATTR(EFI_BLACK, EFI_LIGHTGRAY));

    // Format the first half of the status message
    char_t status[EDITOR_STATUS_MSG_ARR_SIZE];
    int32_t len = snprintf(status, sizeof(status), "%s - %d lines %s",
        cfg.filename != NULL ? cfg.filename : "[No Name]", cfg.numRows,
        cfg.dirty ? "(modified)" : "");

    // Format the second half of the status message
    char_t rstatus[EDITOR_STATUS_MSG_ARR_SIZE];
    int32_t rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d",
        cfg.cy + 1, cfg.numRows);

    if (len >= cfg.editorCols)
    {
        len = cfg.editorCols - 1;
    }
    printf("%s", status); // First half of the status

    while (len < cfg.editorCols - 1)
    {
        // Print the other half of the status (aligned to the right side)
        if (cfg.editorCols - len == rlen)
        {
            printf("%s", rstatus);
            break;
        }
        else // Print spaces to fill the entire line with the background color
        {
            putchar(' ');
        }
        len++;
    }
    // Reset the colors
    ST->ConOut->SetAttribute(ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
}

static void EditorSetStatusMessage(const char_t* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(cfg.statusmsg, sizeof(cfg.statusmsg), fmt, ap);
    va_end(ap);

    // Ensure we don't go out of bounds
    intn_t msglen = strlen(cfg.statusmsg);
    if (msglen >= cfg.editorCols)
    {
        cfg.statusmsg[cfg.editorCols - 1] = CHAR_NULL;
    }
    cfg.statusmsgTime = time(NULL);
}

static void EditorDrawMessageBar(void)
{
    // Fix issue where message bar could be in a bad position
    if (!screenModeSet && ST->ConOut->Mode->CursorColumn != 0)
    {
        putchar('\n');
    }

    // Remove the status message after 5 seconds
    if (cfg.statusmsg[0] != CHAR_NULL &&
        time(NULL) - cfg.statusmsgTime < EDITOR_STATUS_MSG_TIMEOUT)
    {
        printf("%s", cfg.statusmsg);
    }

    // Special buffer for the last row of the screen which is 1 character smaller
    // This is to prevent the cursor from moving down a row and scrolling the screen
    const int32_t size = screenCols - ST->ConOut->Mode->CursorColumn - 1;
    if (size < 1 || !screenModeSet)
    {
        return;
    }

    char_t buf[size];
    memset(buf, ' ', size);
    buf[size - 1] = CHAR_NULL;
    printf("%s", buf);
}

static void EditorInsertRow(int32_t at, char_t* str, size_t len)
{
    if (at < 0 || at > cfg.numRows)
    {
        return;
    }

    // Make room for a new row
    cfg.row = realloc(cfg.row, sizeof(text_row_s) * (cfg.numRows + 1));
    memmove(&cfg.row[at + 1], &cfg.row[at], sizeof(text_row_s) * (cfg.numRows - at));

    // Create the row
    cfg.row[at].size = len;
    cfg.row[at].chars = malloc(len + 1);
    memcpy(cfg.row[at].chars, str, len);
    cfg.row[at].chars[len] = CHAR_NULL;

    cfg.row[at].rsize = 0;
    cfg.row[at].render = NULL;
    EditorUpdateRow(&cfg.row[at]);
    
    cfg.numRows++;
    cfg.dirty = TRUE;
}

static void EditorUpdateRow(text_row_s* row)
{
    // Count the tabs in order to render them as multiple spaces later
    int32_t tabs = 0;
    for (int32_t i = 0; i < row->size; i++)
    {
        if (row->chars[i] == CHAR_TAB)
        {
            tabs++;
        }
    }

    free(row->render);
    row->render = malloc(row->size + tabs*(EDITOR_TAB_SIZE - 1) + 1);

    int32_t idx = 0;
    for (int32_t j = 0; j < row->size; j++)
    {
        // When we reach a tab, add 1 space and then append spaces until a tab stop
        if (row->chars[j] == CHAR_TAB)
        {
            row->render[idx++] = ' ';
            while (idx % EDITOR_TAB_SIZE != 0)
            {
                row->render[idx++] = ' ';
            }
        }
        else
        {
            row->render[idx++] = row->chars[j];
        }
    }
    row->render[idx] = CHAR_NULL;
    row->rsize = idx;
}

// Figure out how many spaces each tab takes up
static intn_t EditorRowCxToRx(text_row_s* row, intn_t cx)
{
    intn_t rx = 0;
    for (int i = 0; i < cx; i++)
    {
        if (row->chars[i] == CHAR_TAB)
        {
            rx += (EDITOR_TAB_SIZE - 1) - (rx % EDITOR_TAB_SIZE);
        }
        rx++;
    }
    return rx;
}

static intn_t EditorRowRxToCx(text_row_s* row, intn_t rx)
{
    intn_t curRx = 0;
    int32_t cx;
    for (cx = 0; cx < row->size; cx++)
    {
        if (row->chars[cx] == CHAR_TAB)
        {
            curRx += (EDITOR_TAB_SIZE - 1) - (curRx % EDITOR_TAB_SIZE);
        }
        curRx++;
        if (curRx > rx)
        {
            return cx;
        }
    }
    return cx;
}

static void EditorRowInsertChar(text_row_s* row, int32_t at, char_t c)
{
    // Validate the index we want to insert a character at
    if (at < 0 || at > row->size)
    {
        at = row->size;
    }

    // Make room for the character we want to insert
    row->chars = realloc(row->chars, row->size + 1);
    memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
    row->size++;
    row->chars[at] = c;

    EditorUpdateRow(row);
    cfg.dirty = TRUE;
}

static void EditorRowDeleteChar(text_row_s* row, int32_t at)
{
    if (at < 0 || at >= row->size)
    {
        return;
    }

    memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
    row->size--;

    EditorUpdateRow(row);
    cfg.dirty = TRUE;
}

static void EditorFreeRow(text_row_s* row)
{
    free(row->render);
    free(row->chars);
}

static void EditorDeleteRow(int32_t at)
{
    if (at < 0 || at >= cfg.numRows)
    {
        return;
    }

    EditorFreeRow(&cfg.row[at]);
    memmove(&cfg.row[at], &cfg.row[at + 1], sizeof(text_row_s) * (cfg.numRows - at - 1));
    cfg.numRows--;
    cfg.dirty = TRUE;
}

static void EditorRowAppendString(text_row_s* row, char_t* str, size_t len)
{
    row->chars = realloc(row->chars, row->size + len + 1);
    memcpy(&row->chars[row->size], str, len);
    row->size += len;
    row->chars[row->size] = CHAR_NULL;

    EditorUpdateRow(row);
    cfg.dirty = TRUE;
}

static void EditorInsertChar(char_t c)
{
    // Ignore unprintable characters (control chars, excluding tabs)
    if (!IsPrintableChar(c) && c != CHAR_TAB)
    {
        return;
    }

    // Append a new line if we write at the end of the file
    if (cfg.cy == cfg.numRows)
    {
        EditorInsertRow(cfg.numRows, "", 0);
    }

    EditorRowInsertChar(&cfg.row[cfg.cy], cfg.cx, c);
    cfg.cx++;
}

static void EditorDeleteChar(void)
{
    // If the cursor is past the end of the file, there's nothing to delete
    if (cfg.cy == cfg.numRows)
    {
        return;
    }
    // If the cursor is at the very beginning of the file, there's nothing to delete
    if (cfg.cx == 0 && cfg.cy == 0)
    {
        return;
    }

    text_row_s* row = &cfg.row[cfg.cy];
    if (cfg.cx > 0)
    {
        EditorRowDeleteChar(row, cfg.cx - 1);
        cfg.cx--;
    }
    else // Handle deleting a character at the beginning of the line
    {
        cfg.cx = cfg.row[cfg.cy - 1].size;
        EditorRowAppendString(&cfg.row[cfg.cy - 1], row->chars, row->size);
        EditorDeleteRow(cfg.cy);
        cfg.cy--;
    }
}

static void EditorInsertNewline(void)
{
    // Insert a new line if we are at the beginning of a line
    if (cfg.cx == 0)
    {
        EditorInsertRow(cfg.cy, "", 0);
    }
    else // Split the line we're on into two rows
    {
        text_row_s* row = &cfg.row[cfg.cy];
        EditorInsertRow(cfg.cy + 1, &row->chars[cfg.cx], row->size - cfg.cx);

        row = &cfg.row[cfg.cy];
        row->size = cfg.cx;
        row->chars[row->size] = CHAR_NULL;
        EditorUpdateRow(row);
    }
    cfg.cy++;
    cfg.cx = 0;
}

// The length parameter is an OUTPUT parameter
static char_t* EditorRowsToString(int32_t* buflen)
{
    // Sum the lengths of each row of text
    int32_t totlen = 0;
    for (int32_t i = 0; i < cfg.numRows; i++)
    {
        // Adding 1 for each newline character which will be added to the end of each line
        totlen += cfg.row[i].size + 1;
    }
    *buflen = totlen;

    char_t* buf = malloc(totlen);
    char_t* p = buf;
    for (int32_t j = 0; j < cfg.numRows; j++)
    {
        // Copy the row contents into the buffer
        memcpy(p, cfg.row[j].chars, cfg.row[j].size);
        p += cfg.row[j].size;
        *p = '\n';
        p++;
    }
    return buf;
}

static void EditorSave(void)
{
    // If we didn't open a file, ask for a file path
    if (cfg.fullFilePath == NULL)
    {
        cfg.fullFilePath = EditorPrompt("Save as: \\%s", NULL);
        if (cfg.fullFilePath == NULL)
        {
            EditorSetStatusMessage("Save aborted.");
            return;
        }

        // Store only the filename, see in EditorOpenFile
        char_t* shortFileName = strrchr(cfg.fullFilePath, '\\');
        cfg.filename = shortFileName != NULL ? shortFileName + 1 : cfg.fullFilePath;
    }

    // Convert the rows of text into one big string
    int32_t len = 0;
    char_t* buf = EditorRowsToString(&len);
    if (len == 0)
    {
        EditorSetStatusMessage("Unable to save empty file.");
        return;
    }

    FILE* fp = fopen(cfg.fullFilePath, "w");
    if (fp != NULL)
    {
        size_t ret = fwrite(buf, 1, len, fp);
        if (ret != 0)
        {
            EditorSetStatusMessage("%d bytes written to disk.", len);
            cfg.dirty = FALSE;

            fclose(fp);
            free(buf);
            return;
        }
        fclose(fp);
    }
    const char_t* errStr = GetCommandErrorInfo(errno);
    Log(LL_ERROR, 0, "Failed to save file %s in editor: %s", cfg.fullFilePath, errStr);
    EditorSetStatusMessage("Failed to save: %s", errStr);
    free(buf);
}

static void EditorFind(void)
{
    intn_t savedCx = cfg.cx;
    intn_t savedCy = cfg.cy;
    intn_t savedColOff = cfg.colOffset;
    intn_t savedRowOff = cfg.rowOffset;

    char_t* query = EditorPrompt("Search: %s (Use ESC/Arrows/Enter)", EditorFindCallback);
    if (query != NULL)
    {
        free(query);
    }
    else // Restore the cursor position before starting the search
    {
        cfg.cx = savedCx;
        cfg.cy = savedCy;
        cfg.colOffset = savedColOff;
        cfg.rowOffset = savedRowOff;
    }
}

static void EditorFindCallback(char_t* query, efi_input_key_t key)
{
    // Used to move forward and backward when searching
    static int32_t lastMatch = -1;
    static int32_t direction = 1;

    char_t unicodechar = key.UnicodeChar;
    uint16_t scancode = key.ScanCode;
    if (unicodechar == CHAR_CARRIAGE_RETURN || scancode == ESCAPE_KEY_SCANCODE)
    {
        lastMatch = -1;
        direction = 1;
        return;
    }
    else if (scancode == RIGHT_ARROW_SCANCODE || scancode == DOWN_ARROW_SCANCODE)
    {
        direction = 1;
    }
    else if (scancode == LEFT_ARROW_SCANCODE || scancode == UP_ARROW_SCANCODE)
    {
        direction = -1;
    }
    else
    {
        lastMatch = -1;
        direction = 1;
    }

    if (lastMatch == -1)
    {
        direction = 1;
    }
    // Index of the current row we are searching
    int32_t current = lastMatch;

    for (intn_t i = 0; i < cfg.numRows; i++)
    {
        current += direction;
        // if...else if to wrap around the end of the file to the beginning
        if (current == -1)
        {
            current = cfg.numRows - 1;
        }
        else if (current == cfg.numRows)
        {
            current = 0;
        }

        text_row_s* row = &cfg.row[current];
        char_t* match = strstr(row->render, query);

        if (match != NULL)
        {
            lastMatch = current;
            cfg.cy = current;
            cfg.cx = EditorRowRxToCx(row, match - row->render);
            cfg.rowOffset = cfg.numRows;
            break;
        }
    }
}

void AppendToBuffer(buffer_s* buf, const char_t* str, uint32_t len)
{
    // Resize the string
    char* new = realloc(buf->b, buf->len + len + 1);
    if (new == NULL)
    {
        return;
    }

    // Update the buffer struct with the new bigger string
    memcpy(&new[buf->len], str, len);
    buf->b = new;
    buf->len += len;
    buf->b[buf->len] = CHAR_NULL;
}

void FreeBuffer(buffer_s* buf)
{
    free(buf->b);
}

void PrintBuffer(buffer_s* buf)
{
    // Printing this way allows printing of binary files
    for (int32_t i = 0; i < buf->len; i++)
    {
        if (buf->b[i] == CHAR_LINEFEED)
        {
            // Overwrite the rest of the row
            PadRow();
        }
        else
        {
            putchar(buf->b[i]);
        }
    }
}

// Taken straight from POSIX-UEFI (differs from current strtok_r)
char_t* strtok_r_with_empty_str(char_t *s, const char_t *d, char_t **p)
{
    int c, sc;
    char_t *tok, *sp;

    if(d == NULL || (s == NULL && (s=*p) == NULL)) return NULL;

    c = *s++;
    for(sp = (char_t *)d; (sc=*sp++)!=0;) {
        if(c == sc) { *p=s; *(s-1)=0; return s-1; }
    }

    if (c == 0) { *p=NULL; return NULL; }
    tok = s-1;
    while(1) {
        c = *s++;
        sp = (char_t *)d;
        do {
            if((sc=*sp++) == c) {
                if(c == 0) s = NULL;
                else *(s-1) = 0;
                *p = s;
                return tok;
            }
        } while(sc != 0);
    }
    return NULL;
}
