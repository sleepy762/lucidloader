#include "editor.h"

int8_t StartEditor(void)
{
    ST->ConOut->ClearScreen(ST->ConOut);

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

    // Keep reading and processing input until the editor is closed
    while (ProcessEditorInput(ConInEx));
    
    ST->ConOut->ClearScreen(ST->ConOut);
    return 0;
}

// Return FALSE when we want to stop processing input
boolean_t ProcessEditorInput(efi_simple_text_input_ex_protocol_t* ConInEx)
{
    efi_key_data_t keyData = GetInputKeyData(ConInEx);
    printf("scancode:%d char:%c", keyData.Key.ScanCode, keyData.Key.UnicodeChar);
    printf("\n");
    printf("shift:%d toggle:%d\n\n", keyData.KeyState.KeyShiftState - EFI_SHIFT_STATE_VALID, keyData.KeyState.KeyToggleState - EFI_TOGGLE_STATE_VALID);

    // Close the editor
    if (IsKeyPressedWithLCtrl(keyData, EDITOR_EXIT_KEY)) return FALSE;

    return TRUE;
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
