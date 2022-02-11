#pragma once
#include <uefi.h>
#include "shellutils.h"
#include "shellerr.h"
#include "logger.h"
#include "efiextendedinput.h"

efi_key_data_t GetInputKeyData(efi_simple_text_input_ex_protocol_t* ConInEx);

int8_t StartEditor(void);
