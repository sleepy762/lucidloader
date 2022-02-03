#pragma once

#include <uefi.h>

#define PASS_FILE_PATH ("EFI\\ezboot\\shell_passwd")

boolen_t CheackPassword(char_t password[]);