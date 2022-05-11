#include "lib/e820.h"

// Also taken from EDK2 (OvmfPkg/Csm/LegacyBiosDxe/LegacyBootSupport.c)
efi_acpi_memory_type_t EfiMemoryTypeToE820Type(uint32_t type)
{
    switch (type) 
    {
        case EfiLoaderCode:
        case EfiLoaderData:
        case EfiBootServicesCode:
        case EfiBootServicesData:
        case EfiConventionalMemory:
        //
        // The memory of EfiRuntimeServicesCode and EfiRuntimeServicesData are
        // usable memory for legacy OS, because legacy OS is not aware of EFI runtime concept.
        // In ACPI specification, EfiRuntimeServiceCode and EfiRuntimeServiceData
        // should be mapped to AddressRangeReserved. This statement is for UEFI OS, not for legacy OS.
        //
        case EfiRuntimeServicesCode:
        case EfiRuntimeServicesData:
        return EfiAcpiAddressRangeMemory;

        case EfiPersistentMemory:
        return EfiAddressRangePersistentMemory;

        case EfiACPIReclaimMemory:
        return EfiAcpiAddressRangeACPI;

        case EfiACPIMemoryNVS:
        return EfiAcpiAddressRangeNVS;

        //
        // All other types map to reserved.
        // Adding the code just wastes FLASH space.
        //
        //  case  EfiReservedMemoryType:
        //  case  EfiUnusableMemory:
        //  case  EfiMemoryMappedIO:
        //  case  EfiMemoryMappedIOPortSpace:
        //  case  EfiPalCode:
        //
        default:
        return EfiAcpiAddressRangeReserved;
    }
}
