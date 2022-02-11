#include "editor.h"

int8_t StartEditor(void)
{
    ST->ConOut->ClearScreen(ST->ConOut);

    efi_simple_text_input_ex_protocol_t* ConInEx;
    efi_guid_t extendedInputGuid = EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID;
    efi_status_t status = BS->LocateProtocol(&extendedInputGuid, NULL, (void**)&ConInEx);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to get extended input protocol.");
        return 1;
    }

    while (1)
    {
        efi_key_data_t keyData = GetInputKeyData(ConInEx);
        printf("char:%c scancode:%d\n", keyData.Key.UnicodeChar, keyData.Key.ScanCode);
        printf("shift:%d toggle:%d\n\n", keyData.KeyState.KeyShiftState - EFI_SHIFT_STATE_VALID, keyData.KeyState.KeyToggleState - EFI_TOGGLE_STATE_VALID);

        // close on ctrl+q
        if (keyData.KeyState.KeyShiftState == EFI_SHIFT_STATE_VALID + EFI_LEFT_CONTROL_PRESSED && keyData.Key.UnicodeChar == 'q')
        {
            break;
        }
    }
    
    return 0;
}

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
