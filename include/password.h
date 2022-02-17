#pragma once
#include <uefi.h>
#include "bootutils.h"
#include "encryption.h"

#define MAX_PASS_LEN 16
#define PASS_FILE_PATH ("EFI\\ezboot\\shell_passwd")

boolean_t CheckPassword();
boolean_t EnterPassword();
