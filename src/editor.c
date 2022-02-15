#include "editor.h"

/* Static declarations */
static boolean_t ProcessEditorInput(efi_simple_text_input_ex_protocol_t* ConInEx);
static void EditorDrawRows(buffer_t* buf);
static void EditorMoveCursor(uint16_t scancode);
static void EditorRefreshScreen(void);
static void InitEditorConfig(void);
static int8_t EditorOpenFile(char_t* filename);
static void AppendEditorWelcomeMessage(buffer_t* buf);
static void EditorAppendRow(char_t* str, size_t len);
static void EditorScroll(void);

static editor_config_t cfg;

/* Renderers */
static void EditorDrawRows(buffer_t* buf)
{
    for (intn_t y = 0; y < cfg.screenRows - 1; y++)
    {
        // Add offset to know what row of the file the user is currently at
        intn_t fileRow = y + cfg.rowOffset;
        if (fileRow >= cfg.numRows)
        {
            // Write a welcome message if no file was loaded
            if (cfg.numRows == 0 && y == cfg.screenRows / 3)
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
            int32_t len = cfg.row[fileRow].size - cfg.colOffset;
            if (len < 0)
            {
                len = 0;
            }
            if (len > cfg.screenCols)
            {
                len = cfg.screenCols - 1;
            }
            AppendToBuffer(buf, &cfg.row[fileRow].chars[cfg.colOffset], len);
        }

        if (y < cfg.screenRows - 1)
        {
            AppendToBuffer(buf, "\n", 1);
        }
    }
}

static void EditorMoveCursor(uint16_t scancode)
{
    // Used to limit scrolling to the right
    text_row_t* row = (cfg.cy >= cfg.numRows) ? NULL : &cfg.row[cfg.cy];

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

static void EditorScroll(void)
{
    if (cfg.cy < cfg.rowOffset) // Scroll up
    {
        cfg.rowOffset = cfg.cy;
    }
    if (cfg.cy >= cfg.rowOffset + cfg.screenRows) // Scroll down
    {
        cfg.rowOffset = cfg.cy - cfg.screenRows + 1;
    }

    if (cfg.cx < cfg.colOffset) // Scroll left
    {
        cfg.colOffset = cfg.cx;
    }
    if (cfg.cx >= cfg.colOffset + cfg.screenCols) // Scroll right
    {
        cfg.colOffset = cfg.cx - cfg.screenCols + 1;
    }
}

static void EditorRefreshScreen(void)
{
    buffer_t buf = BUF_INIT;

    EditorScroll();

    EditorDrawRows(&buf);

    ST->ConOut->EnableCursor(ST->ConOut, FALSE);
    ST->ConOut->ClearScreen(ST->ConOut);
    PrintBuffer(&buf);

    // The offset must be subtracted from the cursor offset, otherwise the value refers to the position
    // of the cursor within the text file and not the position on screen
    ST->ConOut->SetCursorPosition(ST->ConOut, cfg.cx - cfg.colOffset, cfg.cy - cfg.rowOffset);
    ST->ConOut->EnableCursor(ST->ConOut, TRUE);

    FreeBuffer(&buf);
}

static void InitEditorConfig(void)
{
    // Query the amount of rows and columns
    efi_status_t status = ST->ConOut->QueryMode(ST->ConOut, ST->ConOut->Mode->Mode, 
        &cfg.screenCols, &cfg.screenRows);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to get console width and height.");
        // Use default values on error
        cfg.screenCols = 80;
        cfg.screenRows = 25;
    }
    cfg.cx = 0;
    cfg.cy = 0;
    cfg.numRows = 0;
    cfg.rowOffset = 0;
    cfg.colOffset = 0;
    cfg.row = NULL;
}

int8_t StartEditor(char_t* filename)
{
    // We need to get the extended text input protocol in order to get more data about key presses
    // for example, to tell if the left control key was pressed with another key 
    efi_simple_text_input_ex_protocol_t* ConInEx;
    efi_guid_t extendedInputGuid = EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID;
    efi_status_t status = BS->LocateProtocol(&extendedInputGuid, NULL, (void**)&ConInEx);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to get extended input protocol.");
        return -1;
    }

    InitEditorConfig();
    
    if (filename != NULL)
    {
        int8_t res = EditorOpenFile(filename);
        if (res != 0)
        {
            return res;
        }
    }

    // Keep reading and processing input until the editor is closed
    while (ProcessEditorInput(ConInEx));
    
    ST->ConOut->ClearScreen(ST->ConOut);
    return 0;
}

// Return FALSE when we want to stop processing input
static boolean_t ProcessEditorInput(efi_simple_text_input_ex_protocol_t* ConInEx)
{
    EditorRefreshScreen();

    efi_key_data_t keyData = GetInputKeyData(ConInEx);

    // Close the editor
    if (IsKeyPressedWithLCtrl(keyData, EDITOR_EXIT_KEY)) return FALSE;

    switch (keyData.Key.ScanCode)
    {
        // Move the cursor to the top or the bottom of the screen
        case PAGEUP_KEY_SCANCODE:
        case PAGEDOWN_KEY_SCANCODE:
            int32_t times = cfg.screenRows;
            while (times--)
            {
                EditorMoveCursor(keyData.Key.ScanCode == PAGEUP_KEY_SCANCODE ? 
                    UP_ARROW_SCANCODE : DOWN_ARROW_SCANCODE);
            }
            break;

        // Move the cursor to the end or the beginning of the row
        case HOME_KEY_SCANCODE:
            cfg.cx = 0;
            break;
        case END_KEY_SCANCODE:
            cfg.cx = cfg.screenCols - 1;
            break;

        case DELETE_KEY_SCANCODE:
            // Nothing for now
            break;

        // Move the cursor one column/row
        case UP_ARROW_SCANCODE:
        case DOWN_ARROW_SCANCODE:
        case RIGHT_ARROW_SCANCODE:
        case LEFT_ARROW_SCANCODE:
            EditorMoveCursor(keyData.Key.ScanCode);
            break;
    }

    return TRUE;
}

static int8_t EditorOpenFile(char_t* filename)
{
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
    {
        return errno;
    }

    char_t* origDataPtr = GetFileContent(filename);
    char_t* fileData = origDataPtr;
    char_t* token;

    while ((token = strtok_r(fileData, "\n", &fileData)) != NULL)
    {
        size_t linelen = strlen(token);
        while (linelen > 0 && (token[linelen - 1] == '\n' || token[linelen - 1] == '\r'))
        {
            linelen--;
        }
        EditorAppendRow(token, linelen);
    }

    BS->FreePool(origDataPtr);
    fclose(fp);
    return 0;
}

static void EditorAppendRow(char_t* str, size_t len)
{
    cfg.row = realloc(cfg.row, sizeof(text_row_t) * (cfg.numRows + 1));

    int32_t at = cfg.numRows;
    cfg.row[at].size = len;
    cfg.row[at].chars = malloc(len + 1);
    memcpy(cfg.row[at].chars, str, len);
    cfg.row[at].chars[len] = CHAR_NULL;
    cfg.numRows++;
}

static void AppendEditorWelcomeMessage(buffer_t* buf)
{
    // Add our welcome message into a buffer
    char welcome[80];
    int32_t welcomelen = snprintf(welcome, sizeof(welcome), "EZBoot Editor");
    if (welcomelen > cfg.screenCols)
    {
        welcomelen = cfg.screenCols;
    }

    // Add padding to center the welcome message
    intn_t padding = (cfg.screenCols - welcomelen) / 2;
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

// This function is pretty much the same as GetInputKey but using the extended input protocol
efi_key_data_t GetInputKeyData(efi_simple_text_input_ex_protocol_t* ConInEx)
{
    uintn_t idx;

    DisableWatchdogTimer();
    BS->WaitForEvent(1, &ConInEx->WaitForKeyEx, &idx);

    efi_key_data_t keyData = {0};
    efi_status_t status = ConInEx->ReadKeyStrokeEx(ConInEx, &keyData);
    if (EFI_ERROR(status) && status != EFI_NOT_READY)
    {
        Log(LL_ERROR, status, "Failed to read keystroke.");
    }
    EnableWatchdogTimer(DEFAULT_WATCHDOG_TIMEOUT);

    return keyData;
}

boolean_t IsKeyPressedWithLCtrl(efi_key_data_t keyData, char_t key)
{
    return (keyData.KeyState.KeyShiftState == EFI_SHIFT_STATE_VALID + EFI_LEFT_CONTROL_PRESSED 
        && keyData.Key.UnicodeChar == key);
}

void AppendToBuffer(buffer_t* buf, const char_t* str, uint32_t len)
{
    // Resize the string
    char* new = realloc(buf->b, buf->len + len);
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

void FreeBuffer(buffer_t* buf)
{
    free(buf->b);
}

void PrintBuffer(buffer_t* buf)
{
    // This is a worse alternative to using write() which unfortunately doesn't exist here
    for (int32_t i = 0; i < buf->len; i++)
    {
        putchar(buf->b[i]);
    }
}
