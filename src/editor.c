#include "editor.h"

// Editor implementation inspired by snaptoken's kilo
// https://github.com/snaptoken/kilo-src

/*** Static declarations ***/

/* Row operations */
static void EditorInsertRow(int32_t at, char_t* str, size_t len);
static void EditorUpdateRow(text_row_t* row);
static intn_t EditorRowCxToRx(text_row_t* row, intn_t cx);
static intn_t EditorRowRxToCx(text_row_t* row, intn_t rx);
static void EditorRowInsertChar(text_row_t* row, int32_t at, char_t c);
static void EditorRowDeleteChar(text_row_t* row, int32_t at);
static void EditorFreeRow(text_row_t* row);
static void EditorDeleteRow(int32_t at);
static void EditorRowAppendString(text_row_t* row, char_t* str, size_t len);

/* Editor operations */
static void EditorInsertChar(char_t c);
static void EditorDeleteChar(void);
static void EditorInsertNewline(void);

/* File I/O */
static void EditorOpenFile(char_t* filename);
static char_t* EditorRowsToString(int32_t* buflen);
static void EditorSave(void);

/* Search */
static void EditorFind(void);
static void EditorFindCallback(char_t* query, efi_input_key_t key);

/* Output */
static void AppendEditorWelcomeMessage(buffer_t* buf);
static void EditorRefreshScreen(void);
static void EditorDrawRows(buffer_t* buf);
static void EditorScroll(void);
static void EditorDrawStatusBar(void);
static void EditorSetStatusMessage(const char_t* fmt, ...);
static void EditorDrawMessageBar(void);

/* Input */
static boolean_t ProcessEditorInput(void);
static void EditorMoveCursor(uint16_t scancode);
static char_t* EditorPrompt(const char_t* prompt, void (*callback)(char_t*, efi_input_key_t));

/* Init */
static void InitEditorConfig(void);
static editor_config_t cfg;

static void FreeEditorMemory(void);

int8_t StartEditor(char_t* filename)
{
    InitEditorConfig();
    
    if (filename != NULL)
    {
        EditorOpenFile(filename);
    }

    EditorSetStatusMessage(EDITOR_INITIAL_STATUS_MSG);

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
    cfg.fullFilePath = NULL;
    cfg.filename = NULL;
    cfg.statusmsg[0] = CHAR_NULL;
    cfg.statusmsgTime = 0;
    cfg.dirty = FALSE;
}

static void EditorOpenFile(char_t* filename)
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
        return;
    }

    char_t* origDataPtr = GetFileContent(filename);
    char_t* fileData = origDataPtr;
    char_t* token;

    // Read the file line by line
    while ((token = strtok_r(fileData, "\n", &fileData)) != NULL)
    {
        size_t linelen = strlen(token);
        while (linelen > 0 && (token[linelen - 1] == '\n' || token[linelen - 1] == '\r'))
        {
            linelen--;
        }
        EditorInsertRow(cfg.numRows, token, linelen);
    }
    cfg.dirty = FALSE;

    BS->FreePool(origDataPtr);
    fclose(fp);
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
                // CONTROL KEYS
                case EDITOR_EXIT_CTRL_KEY:
                    // Don't exit from modified files immediately
                    if (cfg.dirty && quitTimes > 0)
                    {
                        EditorSetStatusMessage("WARNING!!! File has unsaved changes. "
                            "Press CTRL-Q %d more times to quit.", quitTimes);
                        quitTimes--;
                        return TRUE;
                    }
                    else
                    {
                        return FALSE;
                    }
                    break;

                case EDITOR_SAVE_CTRL_KEY:
                    EditorSave();
                    break;

                case EDITOR_SEARCH_CTRL_KEY:
                    EditorFind();
                    break;

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
    int32_t len = snprintf(status, sizeof(status), "%s - %d lines %s",
        cfg.filename != NULL ? cfg.filename : "[No Name]", cfg.numRows,
        cfg.dirty ? "(modified)" : "");

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
    vsnprintf(cfg.statusmsg, sizeof(cfg.statusmsg), fmt, ap);
    va_end(ap);

    // Ensure we don't go out of bounds
    intn_t msglen = strlen(cfg.statusmsg);
    if (msglen >= cfg.screenCols)
    {
        cfg.statusmsg[cfg.screenCols - 1] = CHAR_NULL;
    }
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

static void EditorInsertRow(int32_t at, char_t* str, size_t len)
{
    if (at < 0 || at > cfg.numRows)
    {
        return;
    }

    // Make room for a new row
    cfg.row = realloc(cfg.row, sizeof(text_row_t) * (cfg.numRows + 1));
    memmove(&cfg.row[at + 1], &cfg.row[at], sizeof(text_row_t) * (cfg.numRows - at));

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

static void EditorUpdateRow(text_row_t* row)
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
static intn_t EditorRowCxToRx(text_row_t* row, intn_t cx)
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

static intn_t EditorRowRxToCx(text_row_t* row, intn_t rx)
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

static void EditorRowInsertChar(text_row_t* row, int32_t at, char_t c)
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

static void EditorRowDeleteChar(text_row_t* row, int32_t at)
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

static void EditorFreeRow(text_row_t* row)
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
    memmove(&cfg.row[at], &cfg.row[at + 1], sizeof(text_row_t) * (cfg.numRows - at - 1));
    cfg.numRows--;
    cfg.dirty = TRUE;
}

static void EditorRowAppendString(text_row_t* row, char_t* str, size_t len)
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
    // Ignore key presses that return a NULL character (for instance escape or f1-f12, print screen, etc.)
    if (c == CHAR_NULL)
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

    text_row_t* row = &cfg.row[cfg.cy];
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
        text_row_t* row = &cfg.row[cfg.cy];
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
    int32_t len;
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

        text_row_t* row = &cfg.row[current];
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

void AppendToBuffer(buffer_t* buf, const char_t* str, uint32_t len)
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

void FreeBuffer(buffer_t* buf)
{
    free(buf->b);
}

void PrintBuffer(buffer_t* buf)
{
    printf("%s", buf->b);
}
