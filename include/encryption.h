#pragma once
#include <uefi.h>

void Encrypt(uint8_t key[], uint8_t text[]);
void HashString(char_t* str);
