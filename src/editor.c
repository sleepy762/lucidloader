#include "editor.h"

/* Static declarations */
static boolean_t ProcessEditorInput(efi_simple_text_input_ex_protocol_t* ConInEx);
static void EditorDrawRows(buffer_t* buf);
static void EditorMoveCursor(uint16_t scancode);
static void EditorRefreshScreen(void);
static void InitEditorConfig(void);
static int8_t EditorOpen(char_t* filename);

static editor_config_t cfg;

/* Renderers */
static void EditorDrawRows(buffer_t* buf)
{
    for (uint16_t y = 0; y < cfg.screenRows; y++)
    {
        // Temporary welcome message
        if (y >= cfg.numRows)
        {
            if (y == cfg.screenRows / 3)
            {
                char welcome[80];
                uintn_t welcomelen = snprintf(welcome, sizeof(welcome), "EZBoot Editor");
                if (welcomelen > cfg.screenCols)
                {
                    welcomelen = cfg.screenCols;
                }
                uintn_t padding = (cfg.screenCols - welcomelen) / 2;
                if (padding)
                {
                    AppendToBuffer(buf, "~", 1);
                    padding--;
                }
                while (padding--) AppendToBuffer(buf, " ", 1);
                AppendToBuffer(buf, welcome, welcomelen);
            }
            else
            {
                AppendToBuffer(buf, "~", 1);
            }
        }
        else
        {
            uint32_t len = cfg.row.size;
            if (len > cfg.screenCols)
            {
                len = cfg.screenCols;
            }
            AppendToBuffer(buf, cfg.row.chars, len);
        }

        if (y < cfg.screenRows - 1)
        {
            AppendToBuffer(buf, "\n", 1);
        }
    }
}

static void EditorMoveCursor(uint16_t scancode)
{
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
            if (cfg.cy != cfg.screenRows - 1)
            {
                cfg.cy++;
            }
            break;
        case RIGHT_ARROW_SCANCODE:
            if (cfg.cx != cfg.screenCols - 1)
            {
                cfg.cx++;
            }
            break;
        case LEFT_ARROW_SCANCODE:
            if (cfg.cx != 0)
            {
                cfg.cx--;
            }
            break;
    }
}

static void EditorRefreshScreen(void)
{
    buffer_t buf = BUF_INIT;

    ST->ConOut->EnableCursor(ST->ConOut, FALSE);
    ST->ConOut->ClearScreen(ST->ConOut);

    EditorDrawRows(&buf);
    PrintBuffer(&buf);

    ST->ConOut->SetCursorPosition(ST->ConOut, cfg.cx, cfg.cy);
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
        return 1;
    }

    InitEditorConfig();
    EditorOpen(filename);

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
    printf("scancode:%d char:%c", keyData.Key.ScanCode, keyData.Key.UnicodeChar);
    printf("\n");
    printf("shift:%d toggle:%d\n\n", keyData.KeyState.KeyShiftState - EFI_SHIFT_STATE_VALID, keyData.KeyState.KeyToggleState - EFI_TOGGLE_STATE_VALID);

    // Close the editor
    if (IsKeyPressedWithLCtrl(keyData, EDITOR_EXIT_KEY)) return FALSE;

    switch (keyData.Key.ScanCode)
    {
        // Move the cursor to the top or the bottom of the screen
        case PAGEUP_KEY_SCANCODE:
        case PAGEDOWN_KEY_SCANCODE:
            uint32_t times = cfg.screenRows;
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

static int8_t EditorOpen(char_t* filename)
{
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
    {
        return errno;
    }

    fclose(fp);
    return 0;
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
    char_t* b = buf->b;
    for (uint32_t i = 0; i < buf->len; i++)
    {
        putchar(b[i]);
    }
}
