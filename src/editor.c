#include "editor.h"

/* Static declarations */
static void InitEditorConfig(void);
static int8_t EditorOpenFile(char_t* filename);
static boolean_t ProcessEditorInput(efi_simple_text_input_ex_protocol_t* ConInEx);
static void AppendEditorWelcomeMessage(buffer_t* buf);
static void EditorRefreshScreen(void);
static void EditorDrawRows(buffer_t* buf);
static void EditorMoveCursor(uint16_t scancode);
static void EditorScroll(void);
static void EditorDrawStatusBar(void);
static void EditorSetStatusMessage(const char_t* fmt, ...);
static void EditorDrawMessageBar(void);
static void EditorAppendRow(char_t* str, size_t len);
static void EditorUpdateRow(text_row_t* row);
static intn_t EditorRowCxToRx(text_row_t* row, intn_t cx);

static editor_config_t cfg;

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

    EditorSetStatusMessage(EDITOR_INITIAL_STATUS_MSG);

    // Keep reading and processing input until the editor is closed
    do
    {
        EditorRefreshScreen();
    } while (ProcessEditorInput(ConInEx));
    
    ST->ConOut->ClearScreen(ST->ConOut);
    return 0;
}

static void InitEditorConfig(void)
{
    // Query the amount of rows and columns
    efi_status_t status = ST->ConOut->QueryMode(ST->ConOut, ST->ConOut->Mode->Mode, 
        (uintn_t*)&cfg.screenCols, (uintn_t*)&cfg.screenRows);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to get console width and height.");
        // Use default values on error
        cfg.screenCols = 80;
        cfg.screenRows = 25;
    }
    cfg.screenRows -= 2; // Making room for the status messages
    cfg.cx = 0;
    cfg.cy = 0;
    cfg.rx = 0;
    cfg.numRows = 0;
    cfg.rowOffset = 0;
    cfg.colOffset = 0;
    cfg.row = NULL;
    cfg.filename = NULL;
    cfg.statusmsg[0] = CHAR_NULL;
    cfg.statusmsgTime = 0;
}

static int8_t EditorOpenFile(char_t* filename)
{
    free(cfg.filename);
    cfg.filename = strdup(filename);

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

// Return FALSE when we want to stop processing input
static boolean_t ProcessEditorInput(efi_simple_text_input_ex_protocol_t* ConInEx)
{
    efi_key_data_t keyData = GetInputKeyData(ConInEx);

    // Close the editor
    if (IsKeyPressedWithLCtrl(keyData, EDITOR_EXIT_KEY)) return FALSE;

    uint16_t scancode = keyData.Key.ScanCode;
    switch (scancode)
    {
        // Scroll a page up/down
        case PAGEUP_KEY_SCANCODE:
        case PAGEDOWN_KEY_SCANCODE:
            if (scancode == PAGEUP_KEY_SCANCODE)
            {
                cfg.cy = cfg.rowOffset;
            }
            else if (scancode == PAGEDOWN_KEY_SCANCODE)
            {                
                cfg.cy = cfg.rowOffset + cfg.screenRows - 1;
                if (cfg.cy > cfg.numRows)
                {
                    cfg.cy = cfg.numRows;
                }
            }

            intn_t times = cfg.screenRows;
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
            cfg.cx = cfg.row[cfg.cy].size;
            break;

        case DELETE_KEY_SCANCODE:
            // Nothing for now
            break;

        // Move the cursor one column/row
        case UP_ARROW_SCANCODE:
        case DOWN_ARROW_SCANCODE:
        case RIGHT_ARROW_SCANCODE:
        case LEFT_ARROW_SCANCODE:
            EditorMoveCursor(scancode);
            break;
    }

    return TRUE;
}

static void AppendEditorWelcomeMessage(buffer_t* buf)
{
    // Add our welcome message into a buffer
    char_t welcome[EDITOR_WELCOME_MSG_ARR_SIZE];
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

static void EditorRefreshScreen(void)
{
    buffer_t buf = BUF_INIT;

    EditorScroll();
    EditorDrawRows(&buf);

    ST->ConOut->EnableCursor(ST->ConOut, FALSE);
    ST->ConOut->ClearScreen(ST->ConOut);

    PrintBuffer(&buf);
    EditorDrawStatusBar();
    EditorDrawMessageBar();

    // The offset must be subtracted from the cursor offset, otherwise the value refers to the position
    // of the cursor within the text file and not the position on screen
    ST->ConOut->SetCursorPosition(ST->ConOut, cfg.rx - cfg.colOffset, cfg.cy - cfg.rowOffset);
    ST->ConOut->EnableCursor(ST->ConOut, TRUE);

    FreeBuffer(&buf);
}

static void EditorDrawRows(buffer_t* buf)
{
    for (intn_t y = 0; y < cfg.screenRows; y++)
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
            int32_t len = cfg.row[fileRow].rsize - cfg.colOffset;
            if (len < 0)
            {
                len = 0;
            }
            if (len >= cfg.screenCols)
            {
                len = cfg.screenCols - 1;
            }
            AppendToBuffer(buf, &cfg.row[fileRow].render[cfg.colOffset], len);
        }
        AppendToBuffer(buf, "\n", 1);
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
    if (cfg.cy < cfg.numRows)
    {
        cfg.rx = EditorRowCxToRx(&cfg.row[cfg.cy], cfg.cx);
    }

    if (cfg.cy < cfg.rowOffset) // Scroll up
    {
        cfg.rowOffset = cfg.cy;
    }
    if (cfg.cy >= cfg.rowOffset + cfg.screenRows) // Scroll down
    {
        cfg.rowOffset = cfg.cy - cfg.screenRows + 1;
    }

    if (cfg.rx < cfg.colOffset) // Scroll left
    {
        cfg.colOffset = cfg.rx;
    }
    if (cfg.rx >= cfg.colOffset + cfg.screenCols) // Scroll right
    {
        cfg.colOffset = cfg.rx - cfg.screenCols + 1;
    }
}

static void EditorDrawStatusBar(void)
{
    // Sets the colors of the status bar
    ST->ConOut->SetAttribute(ST->ConOut, EFI_TEXT_ATTR(EFI_BLACK, EFI_LIGHTGRAY));

    // Format the first half of the status message
    char_t status[EDITOR_STATUS_MSG_ARR_SIZE];
    int32_t len =  snprintf(status, sizeof(status), "%s - %d lines",
        cfg.filename ? cfg.filename : "[No Name]", cfg.numRows);

    // Format the second half of the status message
    char_t rstatus[EDITOR_STATUS_MSG_ARR_SIZE];
    int32_t rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d",
        cfg.cy + 1, cfg.numRows);

    if (len >= cfg.screenCols)
    {
        len = cfg.screenCols - 1;
    }
    printf("%s", status); // First half of the status

    while (len < cfg.screenCols - 1)
    {
        // Print the other half of the status (aligned to the right side)
        if (cfg.screenCols - 1 - len == rlen)
        {
            printf("%s", rstatus);
            break;
        }
        else // Print spaces to fill the entire line with the background color
        {
            ST->ConOut->OutputString(ST->ConOut, L" ");
        }
        len++;
    }
    // Reset the colors
    ST->ConOut->SetAttribute(ST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
    ST->ConOut->OutputString(ST->ConOut, L"\r\n");
}

static void EditorSetStatusMessage(const char_t* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf((char_t*)cfg.statusmsg, sizeof(cfg.statusmsg), fmt, ap);
    va_end(ap);
    cfg.statusmsgTime = time(NULL);
}

static void EditorDrawMessageBar(void)
{
    // Remove the status message after 5 seconds
    if (cfg.statusmsg[0] != CHAR_NULL &&
        time(NULL) - cfg.statusmsgTime < EDITOR_STATUS_MSG_TIMEOUT)
    {
        printf("%s", cfg.statusmsg);
    }
}

static void EditorAppendRow(char_t* str, size_t len)
{
    cfg.row = realloc(cfg.row, sizeof(text_row_t) * (cfg.numRows + 1));

    int32_t at = cfg.numRows;
    cfg.row[at].size = len;
    cfg.row[at].chars = malloc(len + 1);
    memcpy(cfg.row[at].chars, str, len);
    cfg.row[at].chars[len] = CHAR_NULL;

    cfg.row[at].rsize = 0;
    cfg.row[at].render = NULL;
    EditorUpdateRow(&cfg.row[at]);
    
    cfg.numRows++;
}

static void EditorUpdateRow(text_row_t* row)
{
    // Count the tabs in order to render them as multiple spaces later
    int32_t tabs = 0;
    for (int32_t i = 0; i < row->size; i++)
    {
        if (row->chars[i] == '\t')
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
        if (row->chars[j] == '\t')
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
static intn_t EditorRowCxToRx(text_row_t* row, intn_t cx)
{
    intn_t rx = 0;
    for (int i = 0; i < cx; i++)
    {
        if (row->chars[i] == '\t')
        {
            rx += (EDITOR_TAB_SIZE - 1) - (rx % EDITOR_TAB_SIZE);
        }
        rx++;
    }
    return rx;
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
    printf("%s", buf->b);
}
