#include "lib/acpi.h"
#include <uefi.h>

#define RSDP_SIZE (20)
#define XSDP_SIZE (36)

static uint8_t ACPIChecksum(void* ptr, size_t size)
{
    uint8_t sum = 0;
    uint8_t* _ptr = ptr;
    for (size_t i = 0; i < size; i++)
    {
        sum += _ptr[i];
    }
    return sum;
}

void* GetRSDP(void)
{
    efi_guid_t acpi1Guid = ACPI_TABLE_GUID;
    efi_guid_t acpi2Guid = ACPI_20_TABLE_GUID;
    void* rsdp = NULL;

    for (uintn_t i = 0; i < ST->NumberOfTableEntries; i++)
    {
        efi_configuration_table_t* cfgTable = &ST->ConfigurationTable[i];

        boolean_t isXSDP = memcmp(&cfgTable->VendorGuid, &acpi2Guid, sizeof(efi_guid_t)) == 0;
        boolean_t isRSDP = memcmp(&cfgTable->VendorGuid, &acpi1Guid, sizeof(efi_guid_t)) == 0;

        // Check and validate the RSDP/XSDP
        if (!isXSDP && !isRSDP)
        {
            continue;
        }
        if ((isXSDP && ACPIChecksum(cfgTable->VendorTable, XSDP_SIZE)) != 0 ||
            (isRSDP && ACPIChecksum(cfgTable->VendorTable, RSDP_SIZE)) != 0)
        {
            continue;
        }

        // We want to use the XSDP if it exists. If it wasn't found in this iteration
        // we keep iterating over the config table until it is found. If it doesn't
        // exist then the RSDP will be used.
        if (isXSDP)
        {
            rsdp = (void*)cfgTable->VendorTable;
            break;
        }
        else
        {
            rsdp = (void*)cfgTable->VendorTable;
        }
    }
    return rsdp;
}
