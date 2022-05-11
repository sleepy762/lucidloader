#include "lib/edid.h"
#include "logger.h"

#define EFI_EDID_ACTIVE_PROTOCOL_GUID \
{0xbd8c1056,0x9f36,0x44ec,\
{0x92,0xa8,0xa6,0x33,0x7f,0x81,0x79,0x86}}

#define MIN_EDID_SIZE (128)

efi_edid_active_protocol_t* GetEdidActiveProtocol(void)
{
    efi_edid_active_protocol_t* edid = NULL;

    efi_handle_t* handles = NULL;
	uintn_t handlesSize = 0;
	efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

	efi_status_t status = BS->LocateHandle(ByProtocol, &gopGuid, NULL, &handlesSize, handles);
	if (status != EFI_BUFFER_TOO_SMALL)
	{
		Log(LL_ERROR, status, "Failed to get the size of all GOP handles.");
		return NULL;
	}

	handles = malloc(handlesSize);
    if (handles == NULL)
    {
        Log(LL_ERROR, 0, "Failed to allocate memory for the handles.");
        return NULL;
    }

	status = BS->LocateHandle(ByProtocol, &gopGuid, NULL, &handlesSize, handles);
	if (EFI_ERROR(status))
	{
		Log(LL_ERROR, status, "Failed to get the GOP handles.");
        goto end;
	}

    efi_guid_t edidGuid = EFI_EDID_ACTIVE_PROTOCOL_GUID;
    status = BS->HandleProtocol(handles[0], &edidGuid, (void**)&edid);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to handle EDID protocol.");
        goto end;
    }

    if (edid->SizeOfEdid < MIN_EDID_SIZE)
    {
        Log(LL_ERROR, 0, "Invalid EDID size.");
        edid = NULL;
        goto end;
    }

end:
    free(handles);
    return edid;
}
