#include "shell.h"

void StartShell(void)
{
    Log(LL_INFO, 0, "Starting the shell.");
    ST->ConOut->ClearScreen(ST->ConOut);
    ST->ConOut->EnableCursor(ST->ConOut, TRUE);
    printf("Welcome to the bootloader shell!\n");
    printf("Type `help` to get a list of commands.\n");

    char* currPath = NULL;
    // 2 is the initial size for the root dir "\" and null string terminator
    efi_status_t status = BS->AllocatePool(LIP->ImageDataType, 2, (void**)&currPath);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to allocate memory for the path during shell initialization.");
    }

    // Initializing the default starting path
    currPath[0] = '\\';
    currPath[1] = '\0';

    ST->ConIn->Reset(ST->ConIn, 0); // Reset the input buffer
    ShellLoop(&currPath);

    // Cleanup
    Log(LL_INFO, 0, "Closing the shell.");
    BS->FreePool(currPath);
    ST->ConOut->EnableCursor(ST->ConOut, FALSE);
    ST->ConOut->ClearScreen(ST->ConOut);
}

void ShellLoop(char** currPathPtr)
{
    while (1)
    {
        char buffer[SHELL_MAX_INPUT] = {0};
        printf("\n> ");

        GetInput(buffer, SHELL_MAX_INPUT);

        if (!strcmp(buffer, SHELL_EXIT_STR))
        {
            break;
        }
        ProcessCommand(buffer, currPathPtr);
    }
}

void GetInput(char buffer[], const int maxInputSize)
{
    short index = 0;

    efi_status_t status;
    efi_input_key_t key;

    while (1)
    {
        // Continuously read input
        while ((status = ST->ConIn->ReadKeyStroke(ST->ConIn, &key)) == EFI_NOT_READY);

        // When enter is pressed, leave the loop to process the input
        if (key.UnicodeChar == CARRIAGE_RETURN) 
        {
            break;
        }

        // Handling backspace
        if (key.UnicodeChar == BACKSPACE)
        {
            if (index > 0) // Dont delete when the buffer is empty
            {
                index--;
                buffer[index] = 0;
                printf("\b \b"); // Destructive backspace
            }
        }
        // Add the character to the buffer as long as there is enough space and if its a valid character
        // The character in the last index must be null to terminate the string
        else if (index < maxInputSize - 1 && key.UnicodeChar != '\0')
        {
            buffer[index] = key.UnicodeChar;
            index++;
            printf("%c", key.UnicodeChar);
        }
    }
}

void ProcessCommand(char buffer[], char** currPathPtr)
{
    char* cmd = NULL;
    char* args = NULL;
    // Store the command and arguments in separate strings
    ParseInput(buffer, &cmd, &args);
    
    if (cmd == NULL)
    {
        return;
    }
    
    const short totalCmds = CommandCount();
    int commandReturn = 0;
    for (short i = 0; i < totalCmds; i++)
    {   
        // Find the right command and execute the command function
        if (strcmp(cmd, commands[i].commandName) == 0)
        {
            commandReturn = commands[i].CommandFunction(args, currPathPtr);
            break;
        }
        else if (i + 1 == totalCmds)
        {
            printf("\nCommand '%s' not found.", cmd);
        }
    }

    // Let the user know if any error has occurred
    if (commandReturn != CMD_SUCCESS)
    {
        PrintCommandError(cmd, commandReturn);
    }

    BS->FreePool(cmd);
    if (args != NULL)
    {
        BS->FreePool(args);
    }
}

void ParseInput(char buffer[], char** cmd, char** args)
{
    size_t bufferLen = strlen(buffer);
    if (bufferLen == 0)
    {
        return;
    }

    buffer = TrimSpaces(buffer);
    size_t argsOffset = 0;
    GetValueOffset(buffer, &argsOffset, SPACE);

    // Use the argsOffset if there are args present
    size_t cmdSize;
    if (argsOffset != 0)
    {
        cmdSize = argsOffset - 1;
    }
    else
    {
        cmdSize = bufferLen + 1;
    }

    efi_status_t status = BS->AllocatePool(LIP->ImageDataType, cmdSize, (void**)cmd);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to allocate memory while parsing shell input.");
    }
    memcpy(*cmd, buffer, cmdSize);
    (*cmd)[cmdSize] = 0; // Terminate the string

    // If there are arguments present...
    if (argsOffset != 0)
    {
        size_t argsLen = bufferLen - argsOffset;

        status = BS->AllocatePool(LIP->ImageDataType, argsLen + 1, (void**)args);
        if (EFI_ERROR(status))
        {
            Log(LL_ERROR, status, "Failed to allocate memory for shell command arguments.");
        }
        memcpy(*args, buffer + argsOffset, argsLen);
        (*args)[argsLen] = 0; // Terminate the string
    }
}
