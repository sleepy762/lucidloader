#pragma once

typedef enum boot_protocol_t
{
    BP_EFI_LAUNCH
} boot_protocol_t;

const char* ProtocolToString(boot_protocol_t protocol);
