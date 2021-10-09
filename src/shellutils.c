#include "shellutils.h"

char* ConcatPaths(char* lhs, char* rhs)
{
    char* newPath = NULL;
    size_t lhsLen = strlen(lhs);
    size_t rhsLen = strlen(rhs);

    efi_status_t status = BS->AllocatePool(LIP->ImageDataType, lhsLen + rhsLen + 2, (void**)&newPath);
    if (EFI_ERROR(status))
        ErrorExit("Failed to allocate memory for path concatenation.", status);
    memcpy(newPath, lhs, lhsLen + 1); // Copy with null terminator

    if (strlen(lhs) > 1) 
        strcat(newPath, "\\");

    strcat(newPath, rhs);

    return newPath;
}

boolean_t isspace(char c)
{
    return (c == ' ' || c == '\t');
}

void RemoveRepeatedChars(char* str, char toRemove)
{
    char* dest = str;

    while (*str != '\0')
    {
        while (*str == toRemove && *(str + 1) == toRemove)
            str++;
        
        *dest++ = *str++;
    }
    *dest = 0;
}

// Normalizes the path by removing "." and ".." directories from the given path
void NormalizePath(char** path)
{
    // count the amount of tokens
    char* copy = *path;
    int tokenAmount = 0;
    while(*copy != '\0')
    {
        if(*copy == DIRECTORY_DELIM)
            tokenAmount++;
        copy++;
    }
    
    // Nothing to normalize
    if (tokenAmount <= 1) return;

    char** tokens = NULL;
    efi_status_t status = BS->AllocatePool(LIP->ImageDataType, tokenAmount * sizeof(char*), (void**)&tokens);
    if (EFI_ERROR(status))
        ErrorExit("Failed to allocate memory while normalizing path.", status);
    tokens[0] = NULL;

    char* token = NULL;
    char* src = strdup(*path);
    char* srcCopy = src + 1;
    int i = 0;
    // Evaluate the path
    while((token = strtok_r(srcCopy, DIRECTORY_DELIM_STR, &srcCopy)))
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
                tokenAmount--;
            else
                tokenAmount = 0;
            if (i > 0) i--;

            if (tokens[i])
            {
                if (tokenAmount > 0) tokenAmount--;
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
    (*path)[1] = 0;
    for(i = 0; i < tokenAmount; i++)
    {
        strcat(*path, tokens[i]);

        if (i + 1 != tokenAmount)
        {
            strcat(*path, "\\");
        }
            
        BS->FreePool(tokens[i]);
    }
    BS->FreePool(tokens);
}

void CleanPath(char** path)
{
    size_t pathLen = strlen(*path);
    char* originalPath = *path;

    // remove leading whitespace
    while(isspace(**path)) (*path)++;

    // remove trailing whitespace
    char* end = originalPath + pathLen - 1;
    while(end > originalPath && isspace(*end)) end--;
    end[1] = 0;

    // Remove duplicate backslashes from the command
    RemoveRepeatedChars(*path, DIRECTORY_DELIM);

    // Remove a backslash from the end if it exists
    size_t lastIndex = strlen(*path) - 1;
    if ((*path)[lastIndex] == DIRECTORY_DELIM && lastIndex + 1 > 1) (*path)[lastIndex] = 0;
}

char* MakeFullPath(char* args, char* currPathPtr, boolean_t* dynMemFlag)
{
    char* fullPath = NULL;

    CleanPath(&args);

    // Check if the path starts from the root dir
    if (args[0] == DIRECTORY_DELIM)
    {
        fullPath = args;
    }
    // if the args are only whitespace
    else if (args[0] == 0)
    {
        return NULL;
    }
    else // Check the concatenated path
    {
        fullPath = ConcatPaths(currPathPtr, args);
        *dynMemFlag = TRUE;
    }
    return fullPath;
}
