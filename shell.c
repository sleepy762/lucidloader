#include "shell.h"

void StartShell(void)
{
    ST->ConOut->ClearScreen(ST->ConOut);
    printf("Welcome to the bootloader shell!\n");
    printf("Type `help` to get a list of commands.\n");
    ST->ConIn->Reset(ST->ConIn, 0);
    ShellLoop();
}

void ShellLoop(void)
{
    do
    {
        printf("\n> ");
    }
    while(GetInputString());

    ST->ConOut->ClearScreen(ST->ConOut);
}

boolean_t GetInputString(void)
{
    char buffer[MAX_INPUT] = {0};
    short index = 0;

    efi_status_t status;
    efi_input_key_t key;

    while(1)
    {
        // Continuously read input
        while ((status = ST->ConIn->ReadKeyStroke(ST->ConIn, &key)) == EFI_NOT_READY);

        // When enter is pressed, leave the loop to process the input
        if (key.UnicodeChar == CARRIAGE_RETURN) break;

        // Handling backspace
        if (key.UnicodeChar == BACKSPACE)
        {
            if(index > 0)
            {
                index--;
                buffer[index] = 0;
                printf("%c", key.UnicodeChar);
            }
        }
        // Add the character to the buffer as long as there is enough space
        else if (index < MAX_INPUT - 1)
        {
            buffer[index] = key.UnicodeChar;
            index++;
            printf("%c", key.UnicodeChar);
        }
    }

    // Leave the shell
    if(!strcmp(buffer, "exit")) return 0;

    ProcessCommand(buffer);

    return 1;
}

void ProcessCommand(char buffer[])
{
    
}