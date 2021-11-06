#include "cmds/cat.h"

uint8_t CatCmd(char_t args[], char_t** currPathPtr)
{
    if (args == NULL)
    {
        return CMD_NO_FILE_SPECIFIED;
    }

    boolean_t isDynamicMemory = FALSE;

    char_t* filePath = MakeFullPath(args, *currPathPtr, &isDynamicMemory);
    if (filePath == NULL)
    {
        return CMD_NO_FILE_SPECIFIED;
    }

    FILE* file = fopen(filePath, "r");
    if (file != NULL)
    {
        // Get file size
        int64_t fileSize;
        fseek(file, 0, SEEK_END);
        fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Read the file data into a buffer
        char_t* buffer = NULL;
        efi_status_t status = BS->AllocatePool(LIP->ImageDataType, fileSize + 1, (void**)&buffer);
        if (EFI_ERROR(status))
        {
            return CMD_OUT_OF_MEMORY;
        }
        fread(buffer, fileSize, 1, file);
        buffer[fileSize] = CHAR_NULL;
        fclose(file);

        printf("\n%s", buffer);
        BS->FreePool(buffer);
    }
    else
    {
        return errno;
    }
    return CMD_SUCCESS;
}

const char_t* CatBrief(void)
{
    return "Print the contents of a file.";
}

const char_t* CatLong(void)
{
    return "Usage: cat <file1> [file2] [file3] ...";
}
