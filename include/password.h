#pragma once
#include <uefi.h>

#define PASS_FILE_PATH ("\\EFI\\lucidloader\\shell_passwd")

boolean_t ShellLoginWithPassword();
boolean_t CreateShellPassword();
