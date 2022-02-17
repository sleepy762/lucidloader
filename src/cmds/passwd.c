#include "cmds/passwd.h"

uint8_t PasswdCmd(cmd_args_s** args, char_t** currPathPtr)
{
    FILE* passFile = fopen(PASS_FILE_PATH, "r");
    if (passFile == NULL) // Open a file if it doesn't exist
    {
        passFile = fopen(PASS_FILE_PATH, "w");
        if (passFile == NULL)
        {
            PrintCommandError("passwd", NULL, errno);
            return CMD_SUCCESS;
        }
    }
    
    uint64_t fileSize;
    fseek(passFile, 0, SEEK_END);
    fileSize = ftell(passFile);

    if(fileSize != 0)
    {
        printf("are you sure you wanna be dumb and overwrite the password you silly goone\n");
        efi_input_key_t ans = GetInputKey();
        
        if(ans.UnicodeChar != 'y' && ans.UnicodeChar != 'Y')
        {
            printf("cool you are not a fucking bitch :)\n");
            return CMD_SUCCESS;
        }
    }
    fclose(passFile);

    if (EnterPassword() == FALSE)
    {
        PrintCommandError("passwd", NULL, errno);
        return CMD_SUCCESS;
    }

    return CMD_SUCCESS;
}

const char_t* PasswdBrief(void)
{
    return "Lock the shell with a password.";
}

const char_t* PasswdLong(void)
{
    return "Usage: passwd\n\
The command is interactive, once run, it will ask for a password (up to 16 characters) to lock the shell with.\n\
The input is hidden in the command.\n\
The command can also be used to set a new password - the new password will overwrite the old one.";
}
