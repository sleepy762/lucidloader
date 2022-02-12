#pragma once
#include <uefi.h>
#include "bootutils.h"
#include "encryption.h"

#define PASS_FILE_PATH ("EFI\\ezboot\\shell_passwd")

boolean_t CheckPassword(char_t password[]);
boolean_t EnterPassword();
