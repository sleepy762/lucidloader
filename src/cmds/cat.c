#include "cmds/cat.h"

uint8_t CatCmd(cmd_args_s** args, char_t** currPathPtr)
{
    if (*args == NULL)
    {
        return CMD_NO_FILE_SPECIFIED;
    }

    // Print the content of each file in the arguments
    cmd_args_s* arg = *args;
    while(arg != NULL)
    {
        boolean_t isDynamicMemory = FALSE;

        char_t* filePath = MakeFullPath(arg->argString, *currPathPtr, &isDynamicMemory);
        if (filePath == NULL)
        {
            return CMD_NO_FILE_SPECIFIED;
        }

        uint8_t res = PrintFileContent(filePath);
        if (res != CMD_SUCCESS)
        {
            PrintCommandError("cat", arg->argString, res);
        }
        
        if (isDynamicMemory)
        {
            BS->FreePool(filePath);
        }
        arg = arg->next;
    }
    return CMD_SUCCESS;
}

uint8_t PrintFileContent(char_t* path)
{
    FILE* file = fopen(path, "r");
    if (file != NULL)
    {
        // Get file size
        int64_t fileSize;
        fseek(file, 0, SEEK_END);
        fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Prevent a nasty bug from happening when calling fread() with size 0
        if (fileSize == 0)
        {
            fclose(file);
            return CMD_SUCCESS;
        }

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

        printf("%s", buffer);
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
