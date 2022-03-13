#include "config.h"

static boolean_t ValidateEntry(boot_entry_s newEntry);
static void AssignValueToEntry(const char_t* key, char_t* value, boot_entry_s* entry);
static void AppendEntry(boot_entry_array_s* bootEntryArr, boot_entry_s* entry);

// Returns a pointer to the head of a linked list of boot entries
// Every pointer in the linked list was allocated dynamically
boot_entry_array_s ParseConfig(void)
{
    boot_entry_array_s bootEntryArr = { NULL, 0 };

    efi_device_path_t* devPath = NULL;
    efi_file_handle_t* rootDir = NULL;
    efi_file_handle_t* configFileHandle = NULL;
    efi_status_t status = GetFileProtocols(CFG_PATH, &devPath, &rootDir, &configFileHandle);
    if (EFI_ERROR(status))
    {
        return bootEntryArr;
    }

    efi_file_info_t configInfo;
    status = GetFileInfo(configFileHandle, &configInfo);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to get config file info.");
        return bootEntryArr;
    }

    char_t* configData = NULL;
    uint64_t configSize = configInfo.FileSize;
    status = ReadFile(configFileHandle, configSize + 1, &configData);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to read the config file.");
        return bootEntryArr;
    }
    configData[configSize] = 0;

    char_t* line;
    char_t* configEntry;
    char_t* srcCopy = configData;

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
            return bootEntryArr;
        }

        memcpy(strippedEntry, srcCopy, len);
        strippedEntry[len] = 0;
        srcCopy += len + strlen(CFG_ENTRY_DELIMITER); // Move the pointer to the next entry block

        char_t* entryCopy = strippedEntry;
        // Gets lines from the blocks of text
        while ((line = strtok_r(entryCopy, CFG_LINE_DELIMITER, &entryCopy)) != NULL)
        {
            char_t* key = NULL;
            char_t* value = NULL;
            ParseKeyValuePair(line, CFG_KEY_VALUE_DELIMITER, &key, &value);

            if (key == NULL || value == NULL)
            {
                BS->FreePool(key);
                BS->FreePool(value);
                continue;
            }
            AssignValueToEntry(key, value, &entry);
            BS->FreePool(key);
        }

        BS->FreePool(strippedEntry);
        if (ValidateEntry(entry))
        {
            AppendEntry(&bootEntryArr, &entry);
        }
    }

    // Handle the last entry
    boot_entry_s entry = {0};
    while ((line = strtok_r(srcCopy, CFG_LINE_DELIMITER, &srcCopy)) != NULL)
    {
        char_t* key = NULL;
        char_t* value = NULL;
        ParseKeyValuePair(line, CFG_KEY_VALUE_DELIMITER, &key, &value);

        if (key == NULL || value == NULL)
        {
            BS->FreePool(key);
            BS->FreePool(value);
            continue;
        }
        AssignValueToEntry(key, value, &entry);
        BS->FreePool(key);
    }
    if (ValidateEntry(entry))
    {
        AppendEntry(&bootEntryArr, &entry);
    }

    BS->FreePool(configData);
    if (bootEntryArr.numOfEntries == 0)
    {
        Log(LL_ERROR, 0, "The configuration file is empty or has incorrect entries.");
    }
    return bootEntryArr;
}

static boolean_t ValidateEntry(boot_entry_s newEntry)
{
    if (strlen(newEntry.name) == 0)
    {
        Log(LL_WARNING, 0, "Ignoring config entry with no name.");
        return FALSE;
    }
    else if (strlen(newEntry.mainPath) == 0)
    {
        Log(LL_WARNING, 0, "Ignoring entry with no main path specified. (entry name: %s)", newEntry.name);
        return FALSE;
    }
    return TRUE;
}

static void AssignValueToEntry(const char_t* key, char_t* value, boot_entry_s* entry)
{
    if (strcmp(key, "name") == 0)
    {
        // Truncate the name if it's too long
        if (strlen(value) > MAX_ENTRY_NAME_LEN)
        {
            value[MAX_ENTRY_NAME_LEN] = CHAR_NULL;
        }
        entry->name = value;
    }
    else if (strcmp(key, "path") == 0) 
    {
        entry->mainPath = value;
    }
    else if (strcmp(key, "args") == 0)
    {
        entry->imgArgs = value;
    }
    else
    {
        Log(LL_WARNING, 0, "Unknown key '%s' in the config file.", key);
    }
}

// Stores the key and value in separate strings, key and value are OUTPUT parameters
void ParseKeyValuePair(char_t* token, const char_t delimiter, char_t** key, char_t** value)
{
    int32_t valueOffset = GetValueOffset(token, delimiter);
    if (valueOffset == -1)
    {
        return;
    }

    size_t tokenLen = strlen(token);
    size_t valueLength = tokenLen - valueOffset;

    efi_status_t status = BS->AllocatePool(LIP->ImageDataType, valueOffset, (void**)key);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to allocate memory for the key string during config line parsing.");
        return;
    }
    status = BS->AllocatePool(LIP->ImageDataType, valueLength + 1, (void**)value);
    if (EFI_ERROR(status))
    {
        Log(LL_ERROR, status, "Failed to allocate memory for the value string during config line parsing.");
        return;
    }

    memcpy(*key, token, valueOffset - 1);
    (*key)[valueOffset - 1] = CHAR_NULL;

    memcpy(*value, token + valueOffset, valueLength);
    (*value)[valueLength] = CHAR_NULL;
}

// Adds an entry to the end of the entries array
static void AppendEntry(boot_entry_array_s* bootEntryArr, boot_entry_s* entry)
{
    bootEntryArr->entries = realloc(bootEntryArr->entries, 
        sizeof(boot_entry_s) * (bootEntryArr->numOfEntries + 1));

    int32_t at = bootEntryArr->numOfEntries;
    bootEntryArr->entries[at].name = entry->name;
    bootEntryArr->entries[at].mainPath = entry->mainPath;
    bootEntryArr->entries[at].imgArgs = entry->imgArgs;

    bootEntryArr->numOfEntries++;
}

void FreeConfigEntries(boot_entry_array_s* entryArr)
{
    for (int32_t i = 0; i < entryArr->numOfEntries; i++)
    {
        BS->FreePool(entryArr->entries[i].name);
        BS->FreePool(entryArr->entries[i].mainPath);
        BS->FreePool(entryArr->entries[i].imgArgs);
    }
    free(entryArr->entries);
}
