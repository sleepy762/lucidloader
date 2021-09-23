#include "bootutils.h"

wchar_t* StringToWideString(char* str)
{
    const size_t size = strlen(str);
    wchar_t* wpath = NULL;
    
    efi_status_t status = BS->AllocatePool(LIP->ImageDataType, size + 1, (void**)&wpath);
    if(EFI_ERROR(status))
        ErrorExit("Failed to allocate memory during string conversion.", status);

    wpath[size] = 0;
    mbstowcs(wpath, str, size);
    return wpath;
}

void GetFileProtocols(wchar_t* path, efi_device_path_t** devPath, efi_file_handle_t** rootDir, efi_file_handle_t** fileHandle)
{
    // Get all the simple file system protocol handles
    efi_guid_t sfsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    uintn_t bufSize = 0;
    efi_handle_t* handles= NULL;
    efi_status_t status = BS->LocateHandle(ByProtocol, &sfsGuid, NULL, &bufSize, handles);
    if (status != EFI_BUFFER_TOO_SMALL)
        ErrorExit("Initial location of the simple file system protocol handles failed.", status);

    //handles = (efi_handle_t*)malloc(bufSize);
    status = BS->AllocatePool(LIP->ImageDataType, bufSize, (void**)&handles);
    if(EFI_ERROR(status))
        ErrorExit("Failed to allocate buffer for handles.", status);
    //if(!handles)
    //    ErrorExit("Out of memory.", EFI_OUT_OF_RESOURCES);

    status = BS->LocateHandle(ByProtocol, &sfsGuid, NULL, &bufSize, handles);
    if (EFI_ERROR(status))
        ErrorExit("Unable to locate the simple file system protocol handles.", status);

    // Find the right protocols
    uintn_t numHandles = bufSize / sizeof(efi_handle_t);
    efi_simple_file_system_protocol_t* sfsProt = NULL;
    efi_guid_t devGuid = EFI_DEVICE_PATH_PROTOCOL_GUID;

    for (uintn_t i = 0; i < numHandles; i++)
    {
        efi_handle_t handle = handles[i];
        status = BS->HandleProtocol(handle, &sfsGuid, (void**)&sfsProt);
        if (EFI_ERROR(status)) 
        {
            if (i + 1 == numHandles) 
                ErrorExit("Unable to obtain the simple file system protocol.", status);
            continue;
        }

        status = BS->HandleProtocol(handle, &devGuid, (void**)devPath);
        if (EFI_ERROR(status))
        {
            if (i + 1 == numHandles)
                ErrorExit("Unable to obtain the device path protocol.", status);
            continue;
        }

        // This is a way to check that we are accessing the right FAT volume
        // It is not perfect since if a file with the same name exists on 2 different FAT volumes
        // The bootloader will load the first instance

        // Open the root volume
        status = sfsProt->OpenVolume(sfsProt, rootDir);
        if (EFI_ERROR(status))
            continue;

        // Get a handle to the file
        status = (*rootDir)->Open((*rootDir), fileHandle, path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
        if (EFI_ERROR(status))
            PrintDebug("Checking another partition for the file...\n");
    }
    BS->FreePool(handles);
    if((*fileHandle) == NULL) 
        ErrorExit("Failed to find the file on the machine.", EFI_NOT_FOUND);
}

efi_status_t GetFileInfo(efi_file_handle_t* fileHandle, efi_file_info_t* fileInfo)
{
    efi_guid_t infGuid = EFI_FILE_INFO_GUID;
    uintn_t size = sizeof(efi_file_info_t);
    return fileHandle->GetInfo(fileHandle, &infGuid, &size, (void*)fileInfo);
}

efi_status_t ReadFile(efi_file_handle_t* fileHandle, uintn_t fileSize, char** buffer)
{
    efi_status_t status = BS->AllocatePool(LIP->ImageDataType, fileSize, (void**)buffer);
    if(EFI_ERROR(status))
        ErrorExit("Failed to allocate memory to read file.", status);
    return fileHandle->Read(fileHandle, &fileSize, (*buffer));
}

int GetValueOffset(char* line, size_t* valueOffset, const char delimiter)
{
    char* curr = line;

    for(; *curr != delimiter; curr++)
        if(*curr == '\0') return 1; // Delimiter not found

    curr++; // Pass the delimiter
    *valueOffset = curr - line;

    return 0;
}

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