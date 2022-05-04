#include "config.h"
#include "logger.h"
#include "bootutils.h"
#include "shellutils.h"
#include "bootmenu.h"
#include "shellerr.h"

// Entries config path
#define CFG_PATH ("\\EFI\\lucidloader\\config.cfg")

#define CFG_LINE_DELIMITER      ("\n")
#define CFG_ENTRY_DELIMITER     ("\n\n")
#define CFG_KEY_VALUE_DELIMITER (':')
#define CFG_COMMENT_CHAR        ('#')

#define MAX_ENTRY_NAME_LEN (70)

#define BOOT_ENTRY_INIT { NULL, NULL, NULL, 0, FALSE, NULL }
#define BOOT_ENTRY_ARR_INIT { NULL, 0 }

#define LINUX_KERNEL_IDENTIFIER_STR ("vmlinuz")
#define STR_TO_SUBSTITUTE_WITH_VERSION ("%v")

#define INITRD_ARG_STR ("initrd=")

/* Basic config parser functions */
static boolean_t AssignValueToEntry(const char_t* key, char_t* value, boot_entry_s* entry);
static boolean_t ValidateEntry(boot_entry_s* newEntry);
static void AppendEntry(boot_entry_array_s* bootEntryArr, boot_entry_s* entry);
static boolean_t EditRuntimeConfig(const char_t* key, char_t* value);

/* Functions related to the "kerneldir" key in the config */
static void PrepareKernelDirEntry(boot_entry_s* entry);
static char_t* GetPathToKernel(const char_t* directoryPath);
static char_t* GetKernelVersionString(const char_t* fullKernelFileName);

static void FreeConfigEntry(boot_entry_s* entry);
static inline void LogKeyRedefinition(const char_t* key, const char_t* curr, const char_t* ignored);

static void AppendToArgs(boot_entry_s* entry, char_t* value);

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
        strncpy(strippedEntry, filePtr, entryStrLen);

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

            // Get the key and value pair in this line
            char_t* key = NULL;
            char_t* value = NULL;
            if (ParseKeyValuePair(line, CFG_KEY_VALUE_DELIMITER, &key, &value) == FALSE)
            {
                free(key);
                free(value);
                continue;
            }

            // Trim all the spaces before passing into AssignValueToEntry
            const char_t* trimmedKey = TrimSpaces(key);
            char_t* trimmedValue = TrimSpaces(value);
            if (!AssignValueToEntry(trimmedKey, trimmedValue, &entry))
            {
                // Free the value if it wasn't assigned to the entry
                free(value);
            }
            free(key); // Keys are not needed in the end
        }
        free(strippedEntry);

        // Fill the necessary data like kernel path, kernel version and args
        if (entry.isDirectoryToKernel)
        {
            PrepareKernelDirEntry(&entry);
        }

        // Make sure the entry is valid, if it is, then append it to the array of entries
        if (ValidateEntry(&entry))
        {
            AppendEntry(&bootEntryArr, &entry);
        }
        else // Free memory of invalid entry
        {
            FreeConfigEntry(&entry);
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

static boolean_t ValidateEntry(boot_entry_s* newEntry)
{
    // May be a block of comments, don't print warnings in that case
    if (newEntry->name == NULL && newEntry->imgToLoad == NULL && newEntry->imgArgs == NULL)
    {
        return FALSE;
    }

    if (strlen(newEntry->name) == 0)
    {   
        if (!ignoreEntryWarnings)
        {
            Log(LL_WARNING, 0, "Ignoring config entry with no name.");
        }
        return FALSE;
    }
    else if (strlen(newEntry->imgToLoad) == 0)
    {
        if (!ignoreEntryWarnings)
        {
            Log(LL_WARNING, 0, "Ignoring entry with no 'path' or 'kerneldir' specified. (entry name: %s)", newEntry->name);
        }
        return FALSE;
    }
    return TRUE;
}

// Adds a string to the entry args. A space is appended if args aren't NULL
static void AppendToArgs(boot_entry_s* entry, char_t* value)
{
    size_t argsLen = strlen(entry->imgArgs);
    size_t valueLen = strlen(value);
    
    // Resize the args
    size_t newSize = argsLen + valueLen + 1;
    entry->imgArgs = realloc(entry->imgArgs, newSize);

    // Make sure to add a null character if there are no args yet
    if (argsLen == 0)
    {
        entry->imgArgs[argsLen] = CHAR_NULL;
    }
    else // Add a space to separate arguments
    {
        entry->imgArgs[argsLen] = ' ';
        argsLen++;
        entry->imgArgs[argsLen] = CHAR_NULL;
    }
    // Append the new arg
    strncpy(entry->imgArgs + argsLen, value, valueLen);
}

// FALSE means the value wasn't assigned and should be freed (not necessarily failed)
// TRUE means that value is in use and should not be freed
static boolean_t AssignValueToEntry(const char_t* key, char_t* value, boot_entry_s* entry)
{
    // Ignore empty values
    if (value[0] == CHAR_NULL)
    {
        Log(LL_WARNING, 0, "Ignoring empty value given to key '%s'.", key);
        return FALSE;
    }

    // Name of the entry in the menu
    if (strcmp(key, "name") == 0)
    {
        if (entry->name != NULL)
        {
            LogKeyRedefinition(key, entry->name, value);
            return FALSE;
        }

        // Truncate the name if it's too long
        if (strlen(value) > MAX_ENTRY_NAME_LEN)
        {
            value[MAX_ENTRY_NAME_LEN] = CHAR_NULL;
        }
        entry->name = value;
    }
    // Absolute path to a specific file to load
    else if (strcmp(key, "path") == 0) 
    {
        if (entry->isDirectoryToKernel)
        {
            Log(LL_WARNING, 0, "'%s' and 'kerneldir' defined in the same entry. (where kerneldir=%s)",
                key, entry->kernelScanInfo->kernelDirectory);
            return FALSE;
        }
        if (entry->imgToLoad != NULL)
        {
            LogKeyRedefinition(key, entry->imgToLoad, value);
            return FALSE;
        }
        entry->imgToLoad = value;
    }
    // Absolute path to a directory with a Linux kernel
    else if (strcmp(key, "kerneldir") == 0)
    {
        if (entry->imgToLoad != NULL)
        {
            Log(LL_WARNING, 0, "'%s' and 'path' are defined in the same entry. (where path=%s)",
                key, entry->imgToLoad);
            return FALSE;
        }
        if (entry->isDirectoryToKernel)
        {
            LogKeyRedefinition(key, entry->kernelScanInfo->kernelDirectory, value);
            return FALSE;
        }

        entry->kernelScanInfo = malloc(sizeof(kernel_scan_info_s));
        entry->kernelScanInfo->kernelDirectory = value;
        entry->isDirectoryToKernel = TRUE;
    }
    // Concatenates args
    else if (strcmp(key, "args") == 0)
    {
        AppendToArgs(entry, value);
        return FALSE;
    }
    // This key simplifies the configuration but it just takes the value and adds it to the args (for now)
    else if (strcmp(key, "initrd") == 0)
    {
        size_t initrdLen = strlen(INITRD_ARG_STR);
        size_t valueLen = strlen(value);
        size_t totalLen = initrdLen + valueLen + 1;

        // Create the full arg string 'initrd=<value>'
        char_t argStr[totalLen];
        strncpy(argStr, INITRD_ARG_STR, initrdLen);
        strncpy(argStr + initrdLen, value, valueLen);

        AppendToArgs(entry, argStr);
        return FALSE;
    }
    // Set a boot protocol
    else if (strcmp(key, "protocol") == 0)
    {
        // This protocol is used by default and doesn't have to be explicitly stated
        if (strcmp(value, "efilaunch") == 0)
        {
            entry->bootProtocol = BP_EFI_LAUNCH;
        }
        else
        {
            Log(LL_WARNING, 0, "Unknown boot protocol `%s`.", value);
        }
        // multiboot2, stivale2, linux TBD
        return FALSE;
    }
    else // Check if it's a runtime key
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
        return FALSE;
    }
    return TRUE;
}

// Special keys that control the settings of the boot manager during runtime
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
// Return value of FALSE means that there was a failure and the caller may have to free the key or value
boolean_t ParseKeyValuePair(char_t* token, const char_t delimiter, char_t** key, char_t** value)
{
    int32_t valueOffset = GetValueOffset(token, delimiter);
    if (valueOffset == -1)
    {
        return FALSE;
    }

    size_t tokenLen = strlen(token);
    size_t valueLength = tokenLen - valueOffset;
    // Ignore empty lines or pairs with no value
    if (tokenLen == 0 || valueLength == 0)
    {
        return FALSE;
    }

    *key = malloc(valueOffset);
    if (*key == NULL)
    {
        Log(LL_ERROR, 0, "Failed to allocate memory for the key string.");
        return FALSE;
    }
    *value = malloc(valueLength + 1);
    if (*value == NULL)
    {
        Log(LL_ERROR, 0, "Failed to allocate memory for the value string.");
        return FALSE;
    }

    strncpy(*key, token, valueOffset - 1);
    strncpy(*value, token + valueOffset, valueLength);
    return TRUE;
}

// Adds an entry to the end of the entries array
static void AppendEntry(boot_entry_array_s* bootEntryArr, boot_entry_s* entry)
{
    bootEntryArr->entries = realloc(bootEntryArr->entries, 
        sizeof(boot_entry_s) * (bootEntryArr->numOfEntries + 1));

    int32_t at = bootEntryArr->numOfEntries;
    boot_entry_s* newEntry = bootEntryArr->entries + at; // The appended entry in the array

    // Copying the values from the static entry to the entry in the array
    newEntry->name = entry->name;
    newEntry->imgToLoad = entry->imgToLoad;
    newEntry->imgArgs = entry->imgArgs;
    newEntry->bootProtocol = entry->bootProtocol;
    newEntry->isDirectoryToKernel = entry->isDirectoryToKernel;

    if (newEntry->isDirectoryToKernel)
    {
        newEntry->kernelScanInfo = entry->kernelScanInfo;
    }
    else
    {
        newEntry->kernelScanInfo = NULL;
    }

    bootEntryArr->numOfEntries++;
}

// Called when entry->isDirectoryToKernel is TRUE to fill in the rest of the entry data
static void PrepareKernelDirEntry(boot_entry_s* entry)
{
    kernel_scan_info_s* scanInfo = entry->kernelScanInfo;

    entry->imgToLoad = GetPathToKernel(scanInfo->kernelDirectory);
    scanInfo->kernelVersionString = GetKernelVersionString(entry->imgToLoad);

    if (scanInfo->kernelVersionString != NULL)
    {
        // Put the version string wherever it's needed in the args
        char_t* newArgs = StringReplace(entry->imgArgs, STR_TO_SUBSTITUTE_WITH_VERSION, 
            scanInfo->kernelVersionString);
        // Replace the old args if the string replacement function succeeded
        if (newArgs != NULL)
        {
            free(entry->imgArgs);
            entry->imgArgs = newArgs;
        }
    }
    else
    {
        Log(LL_ERROR, 0, "Failed to detect kernel version. (kerneldir=%s, kernel=%s)", 
            scanInfo->kernelDirectory, entry->imgToLoad);
    }
}

static char_t* GetPathToKernel(const char_t* directoryPath)
{
    char_t* path = NULL;
    char_t* kernelName = NULL;

    DIR* dir;
    struct dirent* de;
    if ((dir = opendir(directoryPath)) != NULL)
    {
        while ((de = readdir(dir)) != NULL)
        {
            if (strstr(de->d_name, LINUX_KERNEL_IDENTIFIER_STR) != NULL)
            {
                kernelName = de->d_name;
                break;
            }
        }

        if (kernelName == NULL)
        {
            Log(LL_ERROR, 0, "Linux kernel not found in the directory '%s'.", directoryPath);
            closedir(dir);
            return path;
        }
        // Create a full path to the kernel file
        path = ConcatPaths(directoryPath, kernelName);

        closedir(dir);
    }
    else
    {
        Log(LL_ERROR, 0, "Failed to open directory '%s' to kernel: %s", 
            directoryPath, GetCommandErrorInfo(errno));
    }
    return path;
}

static char_t* GetKernelVersionString(const char_t* fullKernelFileName)
{
    char_t* kernelFileName = strrchr(fullKernelFileName, '\\') + 1;

    // Skip past the kernel file name part
    kernelFileName += strlen(LINUX_KERNEL_IDENTIFIER_STR);
    
    // The next character is the version delimiter, like the '-' in `vmlinuz-x.xx.xx`
    char_t versionDelimiter = *kernelFileName;
    if (versionDelimiter == CHAR_NULL)
    {
        return NULL;
    }
    kernelFileName++;

    // Store the pointer to where the version starts for later
    char_t* startOfVersionStr = kernelFileName;

    // Find where the version string ends
    while (*kernelFileName != versionDelimiter && *kernelFileName != CHAR_NULL)
    {
        kernelFileName++;
    }

    // Store the version string in a dynamic buffer
    int32_t versionStrLen = kernelFileName - startOfVersionStr;
    char_t* versionStr = malloc(versionStrLen + 1);
    if (versionStr == NULL)
    {
        return NULL;
    }

    strncpy(versionStr, startOfVersionStr, versionStrLen);
    return versionStr;
}

static inline void LogKeyRedefinition(const char_t* key, const char_t* curr, const char_t* ignored)
{
    Log(LL_WARNING, 0, "Ignoring '%s' redefinition in the same config entry. (current=%s, ignored=%s)", 
        key, curr, ignored);
}

static void FreeConfigEntry(boot_entry_s* entry)
{
    free(entry->name);
    free(entry->imgToLoad);
    free(entry->imgArgs);

    if (entry->isDirectoryToKernel)
    {
        free(entry->kernelScanInfo->kernelDirectory);
        free(entry->kernelScanInfo->kernelVersionString);
        free(entry->kernelScanInfo);
    }
}

void FreeConfigEntries(boot_entry_array_s* entryArr)
{
    for (int32_t i = 0; i < entryArr->numOfEntries; i++)
    {
        FreeConfigEntry(&entryArr->entries[i]);
    }
    free(entryArr->entries);
}
