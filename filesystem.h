#include <uefi.h>

efi_status_t GetRootVolume(efi_handle_t IM, efi_loaded_image_protocol_t** imgProtocol, efi_file_handle_t** rootDir);