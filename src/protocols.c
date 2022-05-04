#include "protocols.h"

const char* ProtocolToString(boot_protocol_t protocol)
{
    switch (protocol)
    {
        case BP_EFI_LAUNCH:
            return "efilaunch";
        
        default:
            return "unknown";
    }
}
