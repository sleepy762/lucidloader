#pragma once
#include <uefi.h>

// Editor implementation inspired by snaptoken's kilo
// https://github.com/snaptoken/kilo-src

// Used in the input processing function
#define UP_ARROW_SCANCODE       (0x01)
#define DOWN_ARROW_SCANCODE     (0x02)
#define RIGHT_ARROW_SCANCODE    (0x03)
#define LEFT_ARROW_SCANCODE     (0x04)

#define PAGEUP_KEY_SCANCODE     (0x09)
#define PAGEDOWN_KEY_SCANCODE   (0x0A)

#define HOME_KEY_SCANCODE       (0x05)
#define END_KEY_SCANCODE        (0x06)

#define DELETE_KEY_SCANCODE     (0x08)

#define ESCAPE_KEY_SCANCODE     (0x17)

#define F1_KEY_SCANCODE         (0x0B)
#define F2_KEY_SCANCODE         (0x0C)

// Generic buffer struct
typedef struct buffer_s
{
    char_t* b;
    int32_t len;
} buffer_s;

int8_t StartEditor(char_t* filename);

void AppendToBuffer(buffer_s* buf, const char_t* str, uint32_t len);
void PrintBuffer(buffer_s* buf);
void FreeBuffer(buffer_s* buf);

// Similar to strtok but returns an empty string between multiple appearances of a delimiter
char_t* strtok_r_with_empty_str(char_t *s, const char_t *d, char_t **p);
