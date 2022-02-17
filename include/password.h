#pragma once
#include <uefi.h>
#include "bootutils.h"
#include "shellutils.h"
#include "encryption.h"
#include "bootmenu.h"

#define MAX_PASS_LEN 16
#define PASS_FILE_PATH ("EFI\\ezboot\\shell_passwd")
#define SLEEP_LENGTH_FOR_BAD_PASS (2)

boolean_t ShellLoginWithPassword();
boolean_t CreateShellPassword();
