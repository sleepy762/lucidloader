#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "shellerr.h"
#include "logger.h"
#include "efiextendedinput.h"

#define EDITOR_EXIT_KEY ('q')

boolean_t IsKeyPressedWithLCtrl(efi_key_data_t keyData, char_t key);

int8_t StartEditor(void);
efi_key_data_t GetInputKeyData(efi_simple_text_input_ex_protocol_t* ConInEx);
boolean_t ProcessEditorInput(efi_simple_text_input_ex_protocol_t* ConInEx);
