#include <uefi.h>

#define MAX_INPUT 128

#define CARRIAGE_RETURN '\r'  // The "character" when you press the enter key
#define BACKSPACE '\b'

void StartShell(void);
void ShellLoop(void);

boolean_t GetInputString(void);
void ProcessCommand(char buffer[]);