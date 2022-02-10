#pragma once
#include <uefi.h>
#include "bootutils.h"
#include "encryption.h"

#define PASS_FILE_PATH ("EFI\\ezboot\\shell_passwd")
#define MAX_FILE_LEN 100
#define ARR_LEN 16

boolean_t CheckPassword(char_t password[]);
