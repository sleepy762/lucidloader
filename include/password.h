#pragma once
#include <uefi.h>

#define PASS_FILE_PATH ("\\EFI\\ezboot\\shell_passwd")

boolean_t ShellLoginWithPassword();
boolean_t CreateShellPassword();
