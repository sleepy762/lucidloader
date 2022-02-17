#include "cmds/passwd.h"

uint8_t PasswdCmd(cmd_args_s** args, char_t** currPathPtr)
{

    efi_input_key_t ans;
    cmd_args_s* arg = *args;

    FILE* passFile = fopen(PASS_FILE_PATH, "r");
    if (passFile == NULL)
    {
        PrintCommandError("passwd", NULL, CMD_GENERAL_FILE_OPENING_ERROR);
        return CMD_SUCCESS;
    }
    
    uint64_t fileSize;
    fseek(passFile, 0, SEEK_END);
    fileSize = ftell(passFile);

    if(fileSize != 0)
    {
        printf("are you sure you wanna be dumb and overwrite the password you silly goone\n")
        ans = GetInputKey();
        
        if(ans.UnicodeChar != 'y' && ans.UnicodeChar != 'Y')
        {
            printf("cool you are not a fucking bitch :)")
        }
    }


    fclose(passFile);

    return EnterPassword();
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
