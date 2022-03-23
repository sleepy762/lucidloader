#include "config.h"
#include "logger.h"
#include "bootutils.h"
#include "shellutils.h"
#include "bootmenu.h"

// Entries config path
#define CFG_PATH ("EFI\\ezboot\\ezboot-config.cfg")

#define CFG_LINE_DELIMITER      ("\n")
#define CFG_ENTRY_DELIMITER     ("\n\n")
#define CFG_KEY_VALUE_DELIMITER ('=')
#define CFG_COMMENT_CHAR        ('#')

#define MAX_ENTRY_NAME_LEN (70)

static boolean_t ValidateEntry(boot_entry_s newEntry);
static void AssignValueToEntry(const char_t* key, char_t* value, boot_entry_s* entry);
static boolean_t EditRuntimeConfig(const char_t* key, char_t* value);
static void AppendEntry(boot_entry_array_s* bootEntryArr, boot_entry_s* entry);

static boolean_t ignoreEntryWarnings;


// Returns a pointer to the head of a linked list of boot entries
// Every pointer in the linked list was allocated dynamically
boot_entry_array_s ParseConfig(void)
{
    Log(LL_INFO, 0, "Parsing config file...");

    boot_entry_array_s bootEntryArr = BOOT_ENTRY_ARR_INIT;

    uint64_t fileSize = 0;
    char_t* configData = GetFileContent(CFG_PATH, &fileSize);
    if (configData == NULL)
    {
        Log(LL_ERROR, 0, "Failed to read config file.");
        return bootEntryArr;
    }

    // Tracks where we currently are in the config
    char_t* filePtr = configData;

    // Gets blocks of text from the config in a loop
    // Once (filePtr >= configData + fileSize) it means that we have finished reading the entire file
    while (filePtr < configData + fileSize)
    {
        boot_entry_s entry = BOOT_ENTRY_INIT;
        ignoreEntryWarnings = FALSE;

        // Gets a pointer to the end of an entry text block
        char_t* configEntryEnd = strstr(filePtr, CFG_ENTRY_DELIMITER);

        size_t entryStrLen = 0;
        size_t ptrIncrement = 0; // Used to increment filePtr
        if (configEntryEnd == NULL) // This means that we have reached the last entry
        {
            entryStrLen = strlen(filePtr);
            ptrIncrement = entryStrLen;
        }
        else
        {
            entryStrLen = configEntryEnd - filePtr;
            ptrIncrement = entryStrLen + strlen(CFG_ENTRY_DELIMITER);
        }

        // Increment the file pointer and skip empty lines
        if (entryStrLen == 0)
        {
            filePtr += ptrIncrement;
            continue;
        }

        // Holds only the current entry text block
        char_t* strippedEntry = malloc(entryStrLen + 1);
        if (strippedEntry == NULL)
        {
            Log(LL_ERROR, 0, "Failed to allocate memory for an entry block.");
            free(configData);
            return bootEntryArr;
        }

        memcpy(strippedEntry, filePtr, entryStrLen);
        strippedEntry[entryStrLen] = CHAR_NULL;

        // We create a copy because we need to keep the original pointer to free it
        // since strtok modifies the pointer
        char_t* entryCopy = strippedEntry;
        char_t* line = NULL;
        // Gets lines from the blocks of text and parses them
        while ((line = strtok_r(entryCopy, CFG_LINE_DELIMITER, &entryCopy)) != NULL)
        {
            // Ignore comments
            if (line[0] == CFG_COMMENT_CHAR)
            {
                continue;
            }
            char_t* key = NULL;
            char_t* value = NULL;
            ParseKeyValuePair(line, CFG_KEY_VALUE_DELIMITER, &key, &value);

            if (key == NULL || value == NULL)
            {
                free(key);
                free(value);
                continue;
            }
            AssignValueToEntry(key, value, &entry);
            free(key);
        }

        free(strippedEntry);
        if (ValidateEntry(entry))
        {
            AppendEntry(&bootEntryArr, &entry);
        }
        filePtr += ptrIncrement; // Move the pointer to the next entry block
    }

    free(configData);
    if (bootEntryArr.numOfEntries == 0)
    {
        Log(LL_ERROR, 0, "The configuration file is empty or has incorrect entries.");
    }
    return bootEntryArr;
}

static boolean_t ValidateEntry(boot_entry_s newEntry)
{
    // May be a block of comments, don't print warnings in that case
    if (newEntry.name == NULL && newEntry.mainPath == NULL && newEntry.imgArgs == NULL)
    {
        return FALSE;
    }

    if (strlen(newEntry.name) == 0)
    {   
        if (!ignoreEntryWarnings)
        {
            Log(LL_WARNING, 0, "Ignoring config entry with no name.");
        }
        return FALSE;
    }
    else if (strlen(newEntry.mainPath) == 0)
    {
        if (!ignoreEntryWarnings)
        {
            Log(LL_WARNING, 0, "Ignoring entry with no main path specified. (entry name: %s)", newEntry.name);
        }
        return FALSE;
    }
    return TRUE;
}

static void AssignValueToEntry(const char_t* key, char_t* value, boot_entry_s* entry)
{
    if (strcmp(key, "name") == 0)
    {
        if (entry->name != NULL)
        {
            Log(LL_WARNING, 0, "Ignoring '%s' value redefinition in the same config entry. (current=%s, ignored=%s)", key, entry->name, value);
            return;
        }

        // Truncate the name if it's too long
        if (strlen(value) > MAX_ENTRY_NAME_LEN)
        {
            value[MAX_ENTRY_NAME_LEN] = CHAR_NULL;
        }
        entry->name = value;
    }
    else if (strcmp(key, "path") == 0) 
    {
        if (entry->mainPath != NULL)
        {
            Log(LL_WARNING, 0, "Ignoring '%s' value redefinition in the same config entry. (current=%s, ignored=%s)", key, entry->mainPath, value);
            return;
        }
        entry->mainPath = value;
    }
    else if (strcmp(key, "args") == 0)
    {
        if (entry->imgArgs != NULL)
        {
            Log(LL_WARNING, 0, "Ignoring '%s' value redefinition in the same config entry. (current=%s, ignored=%s)", key, entry->imgArgs, value);
            return;
        }
        entry->imgArgs = value;
    }
    else
    {
        boolean_t isRuntimeCfgKey = EditRuntimeConfig(key, value);
        if (!isRuntimeCfgKey)
        {
            Log(LL_WARNING, 0, "Unknown key '%s' in the config file.", key);
        }
        else
        {
            // Avoid false warnings when runtime config keys are on their own
            ignoreEntryWarnings = TRUE;
        }
    }
}

// Special keys that control the settings of the bootloader during runtime
static boolean_t EditRuntimeConfig(const char_t* key, char_t* value)
{
    if (strcmp(key, "timeout") == 0)
    {
        bmcfg.timeoutSeconds = atoi(value);
        if (bmcfg.timeoutSeconds == -1)
        {
            bmcfg.timeoutCancelled = TRUE;
        }
        else if (bmcfg.timeoutSeconds == 0)
        {
            bmcfg.bootImmediately = TRUE;
        }
        return TRUE;
    }
    return FALSE;
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

    *key = malloc(valueOffset);
    if (*key == NULL)
    {
        Log(LL_ERROR, 0, "Failed to allocate memory for the key string.");
        return;
    }
    *value = malloc(valueLength + 1);
    if (*value == NULL)
    {
        Log(LL_ERROR, 0, "Failed to allocate memory for the value string.");
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
        free(entryArr->entries[i].name);
        free(entryArr->entries[i].mainPath);
        free(entryArr->entries[i].imgArgs);
    }
    free(entryArr->entries);
}
