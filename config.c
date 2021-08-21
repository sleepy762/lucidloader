#include "config.h"

// Returns a pointer to the head of a linked list of boot entries
// Every pointer in the linked list was allocated dynamically
boot_entry_s* ParseConfig(void)
{
    efi_device_path_t* devPath = NULL;
    efi_file_handle_t* rootDir = NULL;
    efi_file_handle_t* configFileHandle = NULL;
    GetFileProtocols(cfgPath, &devPath, &rootDir, &configFileHandle);

    efi_file_info_t configInfo;
    efi_status_t status = GetFileInfo(configFileHandle, &configInfo);
    if (EFI_ERROR(status))
        ErrorExit("Failed to get config file info.", status);

    char* configData = NULL;
    uint64_t configSize = configInfo.FileSize;
    status = ReadFile(configFileHandle, configSize + 1, &configData);
    configData[configSize] = 0;

    char* line;
    char* configEntry;
    char* srcCopy = configData;
    boot_entry_s* head = NULL;

    // Gets blocks of text from the config
    while((configEntry = strstr(srcCopy, cfgEntryDelimiter)) != NULL)
    {
        boot_entry_s entry = {0};
        size_t len = configEntry - srcCopy;

        char* strippedEntry = NULL; // Holds the current entry block
        BS->AllocatePool(LIP->ImageDataType, len + 1, (void**)&strippedEntry);

        memcpy(strippedEntry, srcCopy, len);
        strippedEntry[len] = 0;
        srcCopy += len + strlen(cfgEntryDelimiter); // Move the pointer to the next entry block

        char* entryCopy = strippedEntry;
        // Gets lines from the blocks of text
        while((line = strtok_r(entryCopy, cfgLineDelimiter, &entryCopy)))
            ParseLine(&entry, line);

        BS->FreePool(strippedEntry);
        ValidateEntry(entry, &head);
    }

    // Handle the last entry
    boot_entry_s entry = {0};
    while ((line = strtok_r(srcCopy, cfgLineDelimiter, &srcCopy)))
        ParseLine(&entry, line);
    ValidateEntry(entry, &head);

    BS->FreePool(configData);
    return head;
}

int GetValueOffset(char* line, size_t* valueOffset)
{
    char* curr = line;

    for(; *curr != keyValueDelimiter; curr++)
        if(*curr == '\0') return 1; // Delimiter not found

    curr++; // Pass the delimiter
    *valueOffset = curr - line;

    return 0;
}

// If the entry is valid then it is added to the entry linked list
void ValidateEntry(boot_entry_s newEntry, boot_entry_s** head)
{
    if(!strlen(newEntry.name))
    {
        printf("[WARNING] Ignoring entry with no name.\n");
        return;
    }
    else if(newEntry.type != Linux && newEntry.type != Chainload)
    {
        printf("[WARNING] Unknown boot type in entry.\n");
        return;
    }
    else if(!strlen(newEntry.mainPath))
    {
        printf("[WARNING] Ignoring entry with no main path specified.\n");
        return;
    }
    boot_entry_s* entry = InitializeEntry();
    *entry = newEntry;
    entry->next = NULL;

    if (*head == NULL)
        *head = entry;
    else
        AppendEntry(*head, entry);
}

void AssignValueToEntry(const char* key, char* value, boot_entry_s* entry)
{
    if (!strcmp(key, "name")) 
        entry->name = value;
    else if (!strcmp(key, "type"))
    {
        if (!strcmp(value, "chainload")) entry->type = Chainload;
        else if (!strcmp(value, "linux")) entry->type = Linux;
    }
    else if (!strcmp(key, "path") || !strcmp(key, "kernel")) 
        entry->mainPath = value;
    else if (!strcmp(key, "args")) 
        entry->linuxValues.kernelArgs = value;
    else if (!strcmp(key, "initrd")) 
        entry->linuxValues.initrdPath = value;
    else 
        printf("[WARNING] Unknown key value (%s) in config file.\n", key);
}

// Stores the key and value in separate strings
void ParseLine(boot_entry_s* entry, char* token)
{
    size_t valueOffset = 0;
    size_t tokenLen = strlen(token);
    if(GetValueOffset(token, &valueOffset)) return;

    size_t valueLength = tokenLen - valueOffset;

    char* key = NULL;
    char* value = NULL;
    BS->AllocatePool(LIP->ImageDataType, valueOffset, (void**)&key);
    BS->AllocatePool(LIP->ImageDataType, valueLength + 1, (void**)&value);

    memcpy(key, token, valueOffset - 1);
    key[valueOffset - 1] = 0;

    memcpy(value, token + valueOffset, valueLength);
    value[valueLength] = 0;

    AssignValueToEntry(key, value, entry);
    BS->FreePool(key);
}

// Creates a new entry pointer
boot_entry_s* InitializeEntry(void)
{
    boot_entry_s* entry = NULL;
    efi_status_t status = BS->AllocatePool(LIP->ImageDataType, sizeof(boot_entry_s), (void**)&entry);
    if (EFI_ERROR(status))
        ErrorExit("Failed to initialize a pointer for an entry.", status);
    entry->name = NULL;
    entry->type = 0;
    entry->mainPath = NULL;
    entry->linuxValues.initrdPath = NULL;
    entry->linuxValues.kernelArgs = NULL;
    entry->next = NULL;
    return entry;
}

// Adds an entry to the end of the entry linked list
void AppendEntry(boot_entry_s* head, boot_entry_s* entry)
{
    boot_entry_s* copy = head;
    while(copy->next != NULL)
        copy = copy->next;
    copy->next = entry;
}