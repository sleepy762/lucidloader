#pragma once
#include <uefi.h>

// Implements the EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL from UEFI Specification 2.9

#define EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID {0xdd9e7534, 0x7762, 0x4698, {0x8c, 0x14, 0xf5, 0x85, 0x17, 0xa6, 0x25, 0xaa}}

#define EFI_SHIFT_STATE_VALID       0x80000000
#define EFI_RIGHT_SHIFT_PRESSED     0x00000001
#define EFI_LEFT_SHIFT_PRESSED      0x00000002
#define EFI_RIGHT_CONTROL_PRESSED   0x00000004
#define EFI_LEFT_CONTROL_PRESSED    0x00000008
#define EFI_RIGHT_ALT_PRESSED       0x00000010
#define EFI_LEFT_ALT_PRESSED        0x00000020
#define EFI_RIGHT_LOGO_PRESSED      0x00000040
#define EFI_LEFT_LOGO_PRESSED       0x00000080
#define EFI_MENU_KEY_PRESSED        0x00000100
#define EFI_SYS_REQ_PRESSED         0x00000200

#define EFI_TOGGLE_STATE_VALID  0x80
#define EFI_KEY_STATE_EXPOSED   0x40
#define EFI_SCROLL_LOCK_ACTIVE  0x01
#define EFI_NUM_LOCK_ACTIVE     0x02
#define EFI_CAPS_LOCK_ACTIVE    0x04

typedef uint8_t efi_key_toggle_state_t;

typedef struct efi_key_state_t {
    uint32_t KeyShiftState;
    efi_key_toggle_state_t KeyToggleState;
} efi_key_state_t;

typedef struct {
    efi_input_key_t Key;
    efi_key_state_t KeyState;
} efi_key_data_t;

typedef efi_status_t (EFIAPI *efi_input_reset_ex_t) (void *This, boolean_t ExtendedVerification);
typedef efi_status_t (EFIAPI *efi_input_read_key_ex_t) (void *This, efi_key_data_t *KeyData);
typedef efi_status_t (EFIAPI *efi_set_state_t) (void *This, efi_key_toggle_state_t *KeyToggleState);
typedef efi_status_t (EFIAPI *efi_key_notify_function_t) (efi_key_data_t *KeyData);
typedef efi_status_t (EFIAPI *efi_register_keystroke_notify_t) (void *This, efi_key_data_t *KeyData, 
    efi_key_notify_function_t KeyNotificationFunction, void **NotifyHandle);
typedef efi_status_t (EFIAPI *efi_unregister_keystroke_notify_t) (void *This, void *NotificationHandle);

typedef struct _efi_simple_text_input_ex_protocol_t{
    efi_input_reset_ex_t Reset;
    efi_input_read_key_ex_t ReadKeyStrokeEx;
    efi_event_t WaitForKeyEx;
    efi_set_state_t SetState;
    efi_register_keystroke_notify_t RegisterKeyNotify;
    efi_unregister_keystroke_notify_t UnregisterKeyNotify;
} efi_simple_text_input_ex_protocol_t;
