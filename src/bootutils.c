#include "bootutils.h"

wchar_t* StringToWideString(char_t* str)
{
    // The size has to be multiplied by the size of wchar_t 
    // because wchar_t is 2 bytes, while char_t is 1 byte
    const size_t size = strlen(str) * sizeof(wchar_t);
    wchar_t* wpath = malloc(size + 1);
    if (wpath == NULL)
    {
        Log(LL_ERROR, 0, "Failed to allocate memory during string conversion.");
        return NULL;
    }

    wpath[size] = 0;
    mbstowcs(wpath, str, size);
    return wpath;
}

// devPath, rootDir and fileHandle are OUTPUT parameters
efi_status_t GetFileProtocols(char_t* path, efi_device_path_t** devPath, efi_file_handle_t** rootDir, efi_file_handle_t** fileHandle)
{
    // Get all the simple file system protocol handles
    efi_guid_t sfsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    uintn_t bufSize = 0;
    efi_handle_t* handles= NULL;
    efi_status_t status = BS->LocateHandle(ByProtocol, &sfsGuid, NULL, &bufSize, handles);
    if (status != EFI_BUFFER_TOO_SMALL)
    {
        Log(LL_ERROR, status, "Initial location of the simple file system protocol handles failed.");
        return status;
    }

    handles = malloc(bufSize);
    if (handles == NULL)
    {
        Log(LL_ERROR, 0, "Failed to allocate buffer for handles.");
        return EFI_OUT_OF_RESOURCES;
    }

    status = BS->LocateHandle(ByProtocol, &sfsGuid, NULL, &bufSize, handles);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Unable to locate the simple file system protocol handles.");
        return status;
    }

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
            {
                Log(LL_ERROR, status, "Failed to obtain the simple file system protocol.");
                return status;
            }
            continue;
        }

        status = BS->HandleProtocol(handle, &devGuid, (void**)devPath);
        if (EFI_ERROR(status))
        {
            if (i + 1 == numHandles)
            {
                Log(LL_ERROR, status, "Failed to obtain the device path protocol.");
                return status;
            }
            continue;
        }

        // This is a way to check that we are accessing the right FAT volume
        // It is not perfect since if a file with the same name exists on 2 different FAT volumes
        // The bootloader will load the first instance

        // Open the root volume
        status = sfsProt->OpenVolume(sfsProt, rootDir);
        if (EFI_ERROR(status))
        {
            continue;
        }

        // Get a handle to the file
        wchar_t* wpath = StringToWideString(path);
        status = (*rootDir)->Open((*rootDir), fileHandle, wpath, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
        if (EFI_ERROR(status))
        {
            Log(LL_INFO, 0, "Checking another partition for the file '%s'...", path);
        }
        free(wpath);
    }
    free(handles);
    if (*fileHandle == NULL)
    {
        Log(LL_ERROR, 0, "Failed to find the file '%s' on the machine.", path);
        return EFI_NOT_FOUND;
    }
    return EFI_SUCCESS;
}

efi_status_t GetFileInfo(efi_file_handle_t* fileHandle, efi_file_info_t* fileInfo)
{
    efi_guid_t infGuid = EFI_FILE_INFO_GUID;
    uintn_t size = sizeof(efi_file_info_t);
    return fileHandle->GetInfo(fileHandle, &infGuid, &size, (void*)fileInfo);
}

// Returns the size of a file in bytes
uint64_t GetFileSize(FILE* file)
{
    if (file == NULL)
    {
        return 0;
    }

    efi_file_info_t info;
    efi_status_t status = GetFileInfo(file, &info);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to get file info when getting file size.");
        return 0;
    }

    return info.FileSize;
}

// The function reads the file content into a dynamically allocated buffer (null terminated)
// The buffer must be freed by the user
// outFileSize is an optional parameter, it will contain the file size
char_t* GetFileContent(char_t* path, uint64_t* outFileSize)
{
    char_t* buffer = NULL;
    FILE* file = fopen(path, "r");
    if (file != NULL)
    {
        // Get file size
        uint64_t fileSize = GetFileSize(file);
        if (outFileSize != NULL)
        {
            *outFileSize = fileSize;
        }

        buffer = malloc(fileSize + 1);
        if (buffer != NULL)
        {
            fread(buffer, 1, fileSize, file);
            buffer[fileSize] = CHAR_NULL;
        }
        else
        {
            Log(LL_ERROR, 0, "Failed to create buffer to read file.");
        }
        fclose(file);
    }
    return buffer;
}

efi_status_t RebootDevice(boolean_t rebootToFirmware)
{
    ST->ConOut->ClearScreen(ST->ConOut);
    
    efi_status_t status = 0;
    if (rebootToFirmware)
    {
        uint64_t newOsIndications = EFI_OS_INDICATIONS_BOOT_TO_FW_UI;

        efi_guid_t global = EFI_GLOBAL_VARIABLE;
        uintn_t oiSize = 0;

        // Get the size required to store the variable (oiSize is an output parameter)
        RT->GetVariable(L"OsIndications", &global, NULL, &oiSize, NULL);

        // Create a buffer with the appropriate size
        uint64_t* currOsIndications = malloc(oiSize);
        if (currOsIndications == NULL)
        {
            Log(LL_ERROR, 0, "Failed to allocate memory pool for variable data.");
            return EFI_OUT_OF_RESOURCES;
        }

        // Get the actual data
        status = RT->GetVariable(L"OsIndications", &global, NULL, &oiSize, (void*)currOsIndications);
        if (currOsIndications == NULL)
        {
            Log(LL_ERROR, status, "Failed to get OsIndications environment variable.");
            return status;
        }

        if (oiSize == sizeof(newOsIndications))
        {
            newOsIndications |= *currOsIndications;
        }

        // Setting the variable with the firmware reboot bit
        status = RT->SetVariable(L"OsIndications", &global, 
        (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS), 
        sizeof(newOsIndications), (void*)&newOsIndications);
        if (EFI_ERROR(status))
        {
            Log(LL_ERROR, status, "Failed to set OsIndications environment variable.");
        }

        Log(LL_INFO, 0, "Rebooting device into firmware settings...");
        status = RT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
    }
    else
    {
        Log(LL_INFO, 0, "Rebooting device...");
        status = RT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
    }

    // Will be reached only if rebooting failed
    Log(LL_ERROR, status, "Failed to reboot!");
    return status;
}

efi_status_t ShutdownDevice(void)
{
    ST->ConOut->ClearScreen(ST->ConOut);

    Log(LL_INFO, 0, "Shutting down device...");
    efi_status_t status = RT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);

    // Will be reached only if shutting down failed
    Log(LL_ERROR, status, "Failed to shutdown!");
    return status;
}

// The unit for 'timeoutms' is milliseconds
// Waits a certain amount of time before returning 
// or returns immediately upon a key press
int32_t WaitForInput(uint32_t timeoutms)
{
    // The first index is for the timer event, and the second is for WaitForKey event
    uintn_t idx;
    efi_event_t events[2];

    efi_event_t* timerEvent = events;  // Index 0 (Timer)
    events[1] = ST->ConIn->WaitForKey; // Index 1 (Input)

    efi_status_t status = BS->CreateEvent(EVT_TIMER, 0, NULL, NULL, timerEvent);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to create timer event.");
        return INPUT_TIMER_ERROR;
    }

    status = BS->SetTimer(*timerEvent, TimerRelative, timeoutms * 10000);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to set timer to %d seconds.", timeoutms);
        return INPUT_TIMER_ERROR;
    }

    status = BS->WaitForEvent(2, events, &idx);
    BS->CloseEvent(*timerEvent);

    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to wait for timer event.");
        return INPUT_TIMER_ERROR;
    }
    else if (idx == 1) // If a key was pressed during the timer
    {
        return INPUT_TIMER_KEY;
    }
    return INPUT_TIMER_TIMEOUT;
}

void DisableWatchdogTimer(void)
{
    efi_status_t status = BS->SetWatchdogTimer(0, 0, 0, NULL);
    if (EFI_ERROR(status))
    {
        Log(LL_WARNING, status, "Failed to disable watchdog.");
    }
}

void EnableWatchdogTimer(uintn_t seconds)
{
    efi_status_t status = BS->SetWatchdogTimer(seconds, 0, 0, NULL);
    if (EFI_ERROR(status))
    {
        Log(LL_WARNING, status, "Failed to set watchdog timeout to %d seconds.", seconds);
    }
}
