#include "password.h"

boolean_t CheackPassword(char_t password[])
{
    char_t buffer[MAX_FILE_LEN];

    GetPass(buffer);

    //enc conventions we set(just use the msg as key)
    enc(password,password);
    
}

void GetPass(char_t buffer[])
{
    FILE* file = fopen(PASS_FILE_PATH, "r");
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
        if (EFI_ERROR(status))
        {
            return CMD_OUT_OF_MEMORY;
        }
        fread(buffer, fileSize, 1, file);
        buffer[fileSize] = CHAR_NULL;
        fclose(file);

        
    }
}