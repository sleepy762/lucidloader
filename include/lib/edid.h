#pragma once
#include <uefi.h>

typedef struct 
{
    uint32_t SizeOfEdid;
    uint8_t* Edid;
} efi_edid_active_protocol_t;

efi_edid_active_protocol_t* GetEdidActiveProtocol(void);
