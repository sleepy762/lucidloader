#pragma once
#include <uefi.h>

// Taken from EDK2 (OvmfPkg/Csm/LegacyBiosDxe/LegacyBiosInterface.h)
typedef enum {
    EfiAcpiAddressRangeMemory       = 1,
    EfiAcpiAddressRangeReserved     = 2,
    EfiAcpiAddressRangeACPI         = 3,
    EfiAcpiAddressRangeNVS          = 4,
    EfiAddressRangePersistentMemory = 7
} efi_acpi_memory_type_t;

efi_acpi_memory_type_t EfiMemoryTypeToE820Type(uint32_t type);
