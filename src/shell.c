#include "shell.h"

// Return value 1 is fatal, otherwise can be ignored
int8_t StartShell(void)
{
    Log(LL_INFO, 0, "Starting the shell.");
    ST->ConOut->ClearScreen(ST->ConOut);
    ST->ConOut->EnableCursor(ST->ConOut, TRUE);
    printf("Welcome to the bootloader shell!\n");
    printf("Type `help` to get a list of commands.\n");

    char_t* currPath = NULL;
    // 2 is the initial size for the root dir "\" and null string terminator
    efi_status_t status = BS->AllocatePool(LIP->ImageDataType, 2, (void**)&currPath);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to allocate memory for the path during shell initialization.");
        return 1;
    }

    // Initializing the default starting path
    currPath[0] = '\\';
    currPath[1] = '\0';

    ST->ConIn->Reset(ST->ConIn, 0); // Reset the input buffer
    
    if (ShellLoop(&currPath) == 1)
    {
        return 1;
    }

    // Cleanup
    Log(LL_INFO, 0, "Closing the shell.");
    BS->FreePool(currPath);
    ST->ConOut->EnableCursor(ST->ConOut, FALSE);
    ST->ConOut->ClearScreen(ST->ConOut);
    return 0;
}

// Return value 1 is fatal, otherwise can be ignored
int8_t ShellLoop(char_t** currPathPtr)
{
    while (TRUE)
    {
        char_t buffer[SHELL_MAX_INPUT] = {0};
        printf("\n> ");

        GetInput(buffer, SHELL_MAX_INPUT);

        if (strcmp(buffer, SHELL_EXIT_STR) == 0)
        {
            break;
        }
        if (ProcessCommand(buffer, currPathPtr) == 1)
        {
            return 1;
        }
    }
    return 0;
}

void GetInput(char_t buffer[], const uint32_t maxInputSize)
{
    uint32_t index = 0;
    efi_input_key_t key;

    while (TRUE)
    {
        // Continuously read input
        key = GetKey();

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

// Return value 1 is fatal, otherwise can be ignored
int8_t ProcessCommand(char_t buffer[], char_t** currPathPtr)
{
    char_t* cmd = NULL;
    char_t* args = NULL;
    // Store the command and arguments in separate strings
    if (ParseInput(buffer, &cmd, &args) == 1)
    {
        return 1;
    }
    
    if (cmd == NULL)
    {
        return 0;
    }
    
    const uint8_t totalCmds = CommandCount();
    uint8_t commandReturn = 0;
    for (uint8_t i = 0; i < totalCmds; i++)
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
        PrintCommandError(cmd, args, commandReturn);
    }

    BS->FreePool(cmd);
    if (args != NULL)
    {
        BS->FreePool(args);
    }
    return 0;
}

// Return value 1 is fatal, otherwise it can be ignored
int8_t ParseInput(char_t buffer[], char_t** cmd, char_t** args)
{
    size_t bufferLen = strlen(buffer);
    if (bufferLen == 0)
    {
        return 0;
    }

    buffer = TrimSpaces(buffer);
    int32_t argsOffset = GetValueOffset(buffer, SPACE);

    // Use the argsOffset if there are args present
    size_t cmdSize;
    if (argsOffset != -1)
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
        return 1;
    }
    memcpy(*cmd, buffer, cmdSize);
    (*cmd)[cmdSize] = 0; // Terminate the string

    // If there are arguments present...
    if (argsOffset != -1)
    {
        size_t argsLen = bufferLen - argsOffset;

        status = BS->AllocatePool(LIP->ImageDataType, argsLen + 1, (void**)args);
        if (EFI_ERROR(status))
        {
            Log(LL_ERROR, status, "Failed to allocate memory for shell command arguments.");
            return 1;
        }
        memcpy(*args, buffer + argsOffset, argsLen);
        (*args)[argsLen] = 0; // Terminate the string
    }
    return 0;
}
