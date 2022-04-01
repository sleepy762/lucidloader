#include "cmds/about.h"
#include "version.h"
#include "bootutils.h"

boolean_t AboutCmd(cmd_args_s** args, char_t** currPathPtr)
{   
    printf("%s v%s\nBuilt with POSIX-UEFI\n\n", EZBOOT_NAME_STR, EZBOOT_VERSION);

    printf("Running on:\n");
    printf("  Firmware: ");
    ST->ConOut->OutputString(ST->ConOut, ST->FirmwareVendor);
    printf(" %d.%02d\n", ST->FirmwareRevision >> 16, ST->FirmwareRevision & ((1 << 16) - 1));

    printf("  EFI Revision %d.%02d\n", ST->Hdr.Revision >> 16, ST->Hdr.Revision & ((1 << 16) - 1));
    return TRUE;
}
const char_t* AboutBrief(void)
{
    return "Shows information about the system and the bootloader.";
}
