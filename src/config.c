#include "config.h"

// Returns a pointer to the head of a linked list of boot entries
// Every pointer in the linked list was allocated dynamically
boot_entry_s* ParseConfig(void)
{
    efi_device_path_t* devPath = NULL;
    efi_file_handle_t* rootDir = NULL;
    efi_file_handle_t* configFileHandle = NULL;
    GetFileProtocols(CFG_PATH, &devPath, &rootDir, &configFileHandle);

    efi_file_info_t configInfo;
    efi_status_t status = GetFileInfo(configFileHandle, &configInfo);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to get config file info.");
        return NULL;
    }

    char_t* configData = NULL;
    uint64_t configSize = configInfo.FileSize;
    status = ReadFile(configFileHandle, configSize + 1, &configData);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to read the config file.");
        return NULL;
    }
    configData[configSize] = 0;

    char_t* line;
    char_t* configEntry;
    char_t* srcCopy = configData;
    boot_entry_s* head = NULL;

    // Gets blocks of text from the config
    while ((configEntry = strstr(srcCopy, CFG_ENTRY_DELIMITER)) != NULL)
    {
        boot_entry_s entry = {0};
        size_t len = configEntry - srcCopy;

        char_t* strippedEntry = NULL; // Holds the current entry block
        status = BS->AllocatePool(LIP->ImageDataType, len + 1, (void**)&strippedEntry);
        if (EFI_ERROR(status))
        {
            Log(LL_ERROR, status, "Failed to allocate memory while parsing config entries.");
            BS->FreePool(configData);
            return head;
        }

        memcpy(strippedEntry, srcCopy, len);
        strippedEntry[len] = 0;
        srcCopy += len + strlen(CFG_ENTRY_DELIMITER); // Move the pointer to the next entry block

        char_t* entryCopy = strippedEntry;
        // Gets lines from the blocks of text
        while ((line = strtok_r(entryCopy, CFG_LINE_DELIMITER, &entryCopy)) != NULL)
        {
            if (ParseLine(&entry, line) == 1)
            {
                BS->FreePool(configData);
                BS->FreePool(strippedEntry);
                return head;
            }
        }

        BS->FreePool(strippedEntry);
        if (ValidateEntry(entry, &head) == 1)
        {
            BS->FreePool(configData);
            return head;
        }
    }

    // Handle the last entry
    boot_entry_s entry = {0};
    while ((line = strtok_r(srcCopy, CFG_LINE_DELIMITER, &srcCopy)) != NULL)
    {
        if (ParseLine(&entry, line) == 1)
        {
            BS->FreePool(configData);
            return head;
        }
    }
    ValidateEntry(entry, &head);

    BS->FreePool(configData);
    return head;
}

// If the entry is valid then it is added to the entry linked list
// Return value 1 is fatal, otherwise it can be ignored.
int8_t ValidateEntry(boot_entry_s newEntry, boot_entry_s** head)
{
    if (strlen(newEntry.name) == 0)
    {
        Log(LL_WARNING, 0, "Ignoring config entry with no name.");
        return 0;
    }
    else if (newEntry.type != BT_LINUX && newEntry.type != BT_CHAINLOAD)
    {
        Log(LL_WARNING, 0, "Ignoring config entry with unknown boot type. (entry name: %s)", newEntry.name);
        return 0;
    }
    else if (strlen(newEntry.mainPath) == 0)
    {
        Log(LL_WARNING, 0, "Ignoring entry with no main path specified. (entry name: %s)", newEntry.name);
        return 0;
    }
    boot_entry_s* entry = InitializeEntry();

    if (entry == NULL)
    {
        return 1;
    }

    *entry = newEntry;
    entry->next = NULL;

    if (*head == NULL)
    {
        *head = entry;
    }
    else
    {
        AppendEntry(*head, entry);
    }
    return 0;
}

void AssignValueToEntry(const char_t* key, char_t* value, boot_entry_s* entry)
{
    if (strcmp(key, "name") == 0)
    {
        entry->name = value;
    }
    else if (strcmp(key, "type") == 0)
    {
        if (strcmp(value, "chainload") == 0)
        {
            entry->type = BT_CHAINLOAD;
        }
        else if (strcmp(value, "linux") == 0)
        {
            entry->type = BT_LINUX;
        }
    }
    else if (strcmp(key, "path") == 0|| strcmp(key, "kernel") == 0) 
    {
        entry->mainPath = value;
    }
    else if (strcmp(key, "args") == 0)
    {
        entry->linuxValues.kernelArgs = value;
    }
    else if (strcmp(key, "initrd") == 0)
    {
        entry->linuxValues.initrdPath = value;
    }
    else
    {
        Log(LL_WARNING, 0, "Unknown key '%s' in the config file.", key);
    }
}

// Stores the key and value in separate strings
// Return value 1 is fatal, otherwise it can be ignored
int8_t ParseLine(boot_entry_s* entry, char_t* token)
{
    efi_status_t status;
    int32_t valueOffset = GetValueOffset(token, CFG_KEY_VALUE_DELIMITER);
    if (valueOffset == -1)
    {
        return 0;
    }

    size_t tokenLen = strlen(token);
    size_t valueLength = tokenLen - valueOffset;

    char_t* key = NULL;
    char_t* value = NULL;
    status = BS->AllocatePool(LIP->ImageDataType, valueOffset, (void**)&key);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to allocate memory for the key string during config line parsing.");
        return 1;
    }
    status = BS->AllocatePool(LIP->ImageDataType, valueLength + 1, (void**)&value);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to allocate memory for the value string during config line parsing.");
        return 1;
    }

    memcpy(key, token, valueOffset - 1);
    key[valueOffset - 1] = 0;

    memcpy(value, token + valueOffset, valueLength);
    value[valueLength] = 0;

    AssignValueToEntry(key, value, entry);
    BS->FreePool(key);
    return 0;
}

// Creates a new entry pointer
boot_entry_s* InitializeEntry(void)
{
    boot_entry_s* entry = NULL;
    efi_status_t status = BS->AllocatePool(LIP->ImageDataType, sizeof(boot_entry_s), (void**)&entry);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to allocate memory during entry node initialization.");
    }
    else
    {
        entry->name = NULL;
        entry->type = 0;
        entry->mainPath = NULL;
        entry->linuxValues.initrdPath = NULL;
        entry->linuxValues.kernelArgs = NULL;
        entry->next = NULL;
    }
    return entry;
}

// Adds an entry to the end of the entry linked list
void AppendEntry(boot_entry_s* head, boot_entry_s* entry)
{
    boot_entry_s* copy = head;
    while (copy->next != NULL)
    {
        copy = copy->next;
    }
    copy->next = entry;
}
