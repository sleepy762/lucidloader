#include "shellutils.h"

char_t* ConcatPaths(char_t* lhs, char_t* rhs)
{
    size_t lhsLen = strlen(lhs);
    size_t rhsLen = strlen(rhs);

    // Null terminator + additional '\' character
    char_t* newPath = malloc(lhsLen + rhsLen + 2);
    if (newPath == NULL)
    {
        Log(LL_ERROR, 0, "Failed to allocate memory to concatenate two paths.");
        return NULL;
    }

    memcpy(newPath, lhs, lhsLen + 1); // Copy with null terminator

    // Don't add an extra backslash if the lhs path is "\"
    if (strlen(lhs) > 1)
    {
        strcat(newPath, "\\");
    }

    strcat(newPath, rhs);

    return newPath;
}

// Normalizes the path by removing "." and ".." directories from the given path
uint8_t NormalizePath(char_t** path)
{
    // count the amount of tokens
    char_t* copy = *path;
    uint16_t tokenAmount = 0;
    while (*copy != CHAR_NULL)
    {
        if (*copy == DIRECTORY_DELIM)
        {
            tokenAmount++;
        }
        copy++;
    }
    
    // Nothing to normalize
    if (tokenAmount <= 1) 
    {
        return CMD_SUCCESS;
    }

    char_t** tokens = malloc(tokenAmount * sizeof(char_t*));
    if (tokens == NULL)
    {
        Log(LL_ERROR, 0, "Failed to allocate memory while normalizing the path.");
        return CMD_OUT_OF_MEMORY;
    }
    tokens[0] = NULL;

    char_t* token = NULL;
    char_t* srcCopy = *path + 1; // Pass the first character (which is always "\")
    uint16_t i = 0;
    // Evaluate the path
    while ((token = strtok_r(srcCopy, DIRECTORY_DELIM_STR, &srcCopy)) != NULL)
    {
        // Ignore the "." directory
        if (strcmp(token, CURRENT_DIR) == 0)
        {
            tokenAmount--;
        }
        // Go backwards in the path
        else if (strcmp(token, PREVIOUS_DIR) == 0)
        {
            if (tokenAmount > 0) 
            {
                tokenAmount--;
            }
            else
            {
                tokenAmount = 0;
            }

            // Don't go backwards past the beginning
            if (i > 0) 
            {
                i--;
            }

            if (tokens[i] != NULL)
            {
                if (tokenAmount > 0) 
                {
                    tokenAmount--;
                }
                free(tokens[i]);
                tokens[i] = NULL;
            }
        }
        else
        {
            tokens[i] = strdup(token);
            i++;
        }
    }

    // Rebuild the string
    (*path)[0] = '\\';
    (*path)[1] = CHAR_NULL;
    for (i = 0; i < tokenAmount; i++)
    {
        strcat(*path, tokens[i]);

        if (i + 1 != tokenAmount)
        {
            strcat(*path, "\\");
        }
            
        free(tokens[i]);
    }
    free(tokens);
    return CMD_SUCCESS;
}

void CleanPath(char_t** path)
{
    *path = TrimSpaces(*path);

    // Remove duplicate backslashes from the command
    RemoveRepeatedChars(*path, DIRECTORY_DELIM);

    // Remove a backslash from the end if it exists
    size_t lastIndex = strlen(*path) - 1;
    if ((*path)[lastIndex] == DIRECTORY_DELIM && lastIndex + 1 > 1) 
    {
        (*path)[lastIndex] = 0;
    }
}

char_t* MakeFullPath(char_t* pathArg, char_t* currPathPtr, boolean_t* isDynamicMemory)
{
    char_t* fullPath = NULL;

    CleanPath(&pathArg);

    // Check if the path starts from the root dir
    if (pathArg[0] == DIRECTORY_DELIM)
    {
        fullPath = pathArg;
        *isDynamicMemory = FALSE;
    }
    // if the args are only whitespace
    else if (pathArg[0] == CHAR_NULL)
    {
        *isDynamicMemory = FALSE;
        return NULL;
    }
    else // Check the concatenated path
    {
        fullPath = ConcatPaths(currPathPtr, pathArg);
        if (fullPath == NULL)
        {
            return NULL;
        }
        *isDynamicMemory = TRUE;
    }
    return fullPath;
}

boolean_t IsPrintableChar(char_t c)
{
    return (c  >= ' ' && c <= '~');
}

boolean_t IsSpace(char_t c)
{
    return (c == ' ' || c == CHAR_TAB);
}

char_t* TrimSpaces(char_t* str)
{
    size_t stringLen = strlen(str);
    char_t* originalString = str;

    // remove leading whitespace
    while (IsSpace(*str))
    {
        str++;
    }

    // remove trailing whitespace
    char_t* end = originalString + stringLen - 1;
    while (end > originalString && IsSpace(*end)) 
    {
        end--;
    }
    end[1] = 0;

    return str;
}

void RemoveRepeatedChars(char_t* str, char_t toRemove)
{
    char_t* dest = str;

    while (*str != CHAR_NULL)
    {
        while (*str == toRemove && *(str + 1) == toRemove)
        {
            str++;
        }
        *dest++ = *str++;
    }
    *dest = 0;
}

efi_input_key_t GetInputKey(void)
{
    efi_status_t status = 0;
    efi_input_key_t key = {0};

    DisableWatchdogTimer();

    // Make sure we are not returning 0 values in the key struct
    do
    {
        uintn_t idx;
        BS->WaitForEvent(1, &ST->ConIn->WaitForKey, &idx);
        status = ST->ConIn->ReadKeyStroke(ST->ConIn, &key);
    } while (EFI_ERROR(status));

    EnableWatchdogTimer(DEFAULT_WATCHDOG_TIMEOUT);
    return key;
}

void GetInputString(char_t buffer[], const uint32_t maxInputSize, boolean_t hideInput)
{
    uint32_t index = 0;

    while (TRUE)
    {
        // Continuously read input
        efi_input_key_t key = GetInputKey();
        char_t unicodechar = key.UnicodeChar;

        // When enter is pressed, leave the loop to process the input
        if (unicodechar == CHAR_CARRIAGE_RETURN) 
        {
            putchar('\n');
            break;
        }

        // Handling backspace
        if (unicodechar == CHAR_BACKSPACE)
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
        else if (index < maxInputSize - 1 && IsPrintableChar(unicodechar))
        {
            buffer[index] = unicodechar;
            index++;

            if (hideInput) // When entering a password
            {
                putchar('*');
            }
            else
            {
                putchar(unicodechar);
            }
        }
    }
}

int32_t GetValueOffset(char_t* line, const char_t delimiter)
{
    char* curr = line;

    for (; *curr != delimiter; curr++)
    {
        if (*curr == CHAR_NULL)
        {
            return -1; // Delimiter not found
        }
    }

    curr++; // Pass the delimiter
    return (curr - line);
}

// Tries to find a flag in the arguments, returns TRUE if it's found, FALSE otherwise
// Additionally it removes the first instance of the node of the flag from the linked list
boolean_t FindFlagAndDelete(cmd_args_s** argsHead, const char* flagStr)
{
    // If there are no args, the flag won't be found
    if (*argsHead == NULL || flagStr == NULL)
    {
        return FALSE;
    }

    cmd_args_s* args = *argsHead;
    // If the requested node is the head, change it to the next pointer
    if (strcmp(args->argString, flagStr) == 0)
    {
        *argsHead = args->next;
        free(args->argString);
        free(args);
        return TRUE;
    }

    // Start from the 2nd node
    cmd_args_s* prev = args;
    args = args->next;
    while (args != NULL)
    {
        // Check if the flag is in the current node
        if (strcmp(args->argString, flagStr) == 0)
        {
            // Deleting the argument node
            prev->next = args->next;
            free(args->argString);
            free(args);
            return TRUE;
        }
        // Advancing the list search
        prev = args;
        args = args->next;
    }
    return FALSE;
}

cmd_args_s* GetLastArg(cmd_args_s* head)
{
    while (head->next != NULL)
    {
        head = head->next;
    }
    return head;
}

int32_t PrintFileContent(char_t* path)
{
    uint64_t fileSize = 0;
    char_t* buffer = GetFileContent(path, &fileSize);
    if (buffer == NULL)
    {
        return errno;
    }

    // Printing the file like this in order to prevent possible issues when reading binary files
    for (uint64_t i = 0; i < fileSize; i++)
    {
        putchar(buffer[i]);
    }
    putchar('\n');

    free(buffer);
    
    return 0;
}

int32_t CopyFile(const char_t* src, const char_t* dest)
{
    FILE* srcFP = fopen(src, "r");
    if (srcFP == NULL)
    {
        return errno;
    }
    FILE* destFP = fopen(dest, "w");
    if (destFP == NULL)
    {
        return errno;
    }

    char_t buf[BUFSIZ];
    uint64_t srcSize = GetFileSize(srcFP);

    for (uint64_t i = 0; i < srcSize; i += BUFSIZ)
    {
        uint64_t bytesToCopy;
        if (i + BUFSIZ > srcSize) // Copy the remainder
        {
            bytesToCopy = srcSize - i;
        }
        else // Copy up to 8KiB at a time
        {
            bytesToCopy = BUFSIZ;
        }

        if (fread(buf, 1, bytesToCopy, srcFP) != bytesToCopy ||
            fwrite(buf, 1, bytesToCopy, destFP) != bytesToCopy)
        {
            Log(LL_ERROR, 0, "Error during file copy: %s", GetCommandErrorInfo(errno));
            return errno;
        }
    }
    fclose(destFP);
    fclose(srcFP);
    return 0;
}

int32_t CreateDirectory(char_t* path)
{
    DIR* dir = opendir(path);
    if (dir != NULL)
    {
        closedir(dir);
        return CMD_DIR_ALREADY_EXISTS;
    }
    else
    {
        // Creates a new directory and frees the pointer to it
        FILE* fp = fopen(path, "wd");
        if (fp != NULL)
        {
            fclose(fp);
        }
        else
        {
            // Make the error message more sensible
            return (errno == ENOTDIR) ? EEXIST : errno;
        }
    }
    return CMD_SUCCESS;
}
