#include "shellutils.h"

char_t* ConcatPaths(char_t* lhs, char_t* rhs)
{
    char_t* newPath = NULL;
    size_t lhsLen = strlen(lhs);
    size_t rhsLen = strlen(rhs);

    efi_status_t status = BS->AllocatePool(LIP->ImageDataType, lhsLen + rhsLen + 2, (void**)&newPath);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to allocate memory to concatenate two paths.");
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
    while (*copy != '\0')
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

    char_t** tokens = NULL;
    efi_status_t status = BS->AllocatePool(LIP->ImageDataType, tokenAmount * sizeof(char_t*), (void**)&tokens);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to allocate memory while normalizing the path.");
        return CMD_OUT_OF_MEMORY;
    }
        
    tokens[0] = NULL;

    char_t* token = NULL;
    // char* src = *path;
    char_t* src = strdup(*path);
    char_t* srcCopy = src + 1;
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
                BS->FreePool(tokens[i]);
                tokens[i] = NULL;
            }
        }
        else
        {
            tokens[i] = strdup(token);
            i++;
        }
    }
    BS->FreePool(src);

    // Rebuild the string
    (*path)[0] = '\\';
    (*path)[1] = '\0';
    for (i = 0; i < tokenAmount; i++)
    {
        strcat(*path, tokens[i]);

        if (i + 1 != tokenAmount)
        {
            strcat(*path, "\\");
        }
            
        BS->FreePool(tokens[i]);
    }
    BS->FreePool(tokens);

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

char_t* MakeFullPath(char_t* args, char_t* currPathPtr, boolean_t* isDynamicMemory)
{
    char_t* fullPath = NULL;

    CleanPath(&args);

    // Check if the path starts from the root dir
    if (args[0] == DIRECTORY_DELIM)
    {
        fullPath = args;
    }
    // if the args are only whitespace
    else if (args[0] == '\0')
    {
        return NULL;
    }
    else // Check the concatenated path
    {
        fullPath = ConcatPaths(currPathPtr, args);
        if (fullPath == NULL)
        {
            return NULL;
        }
        
        *isDynamicMemory = TRUE;
    }
    return fullPath;
}

boolean_t isspace(char_t c)
{
    return (c == ' ' || c == '\t');
}

char_t* TrimSpaces(char_t* str)
{
    size_t stringLen = strlen(str);
    char_t* originalString = str;

    // remove leading whitespace
    while (isspace(*str))
    {
        str++;
    }

    // remove trailing whitespace
    char_t* end = originalString + stringLen - 1;
    while (end > originalString && isspace(*end)) 
    {
        end--;
    }
    end[1] = 0;

    return str;
}

void RemoveRepeatedChars(char_t* str, char_t toRemove)
{
    char_t* dest = str;

    while (*str != '\0')
    {
        while (*str == toRemove && *(str + 1) == toRemove)
        {
            str++;
        }
        *dest++ = *str++;
    }
    *dest = 0;
}
