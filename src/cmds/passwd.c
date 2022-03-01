#include "cmds/passwd.h"

boolean_t PasswdCmd(cmd_args_s** args, char_t** currPathPtr)
{
    cmd_args_s* cmdArg = *args;
    // Check if a password file already exists
    FILE* passFile = fopen(PASS_FILE_PATH, "r");
    if (passFile == NULL) // Open a file if it doesn't exist
    {
        passFile = fopen(PASS_FILE_PATH, "w");
        if (passFile == NULL)
        {
            PrintCommandError(cmdArg->argString, NULL, errno);
            return FALSE;
        }
    }

    uint64_t fileSize = GetFileSize(passFile);
    fclose(passFile);
    
    if(fileSize != 0)
    {
        printf("A password already exists, do you want to overwrite it? (y/n) ");
        efi_input_key_t ans = GetInputKey();
        
        putchar('\n');
        if(ans.UnicodeChar != 'y' && ans.UnicodeChar != 'Y')
        {
            return TRUE;
        }
    }

    if (CreateShellPassword() == FALSE)
    {
        PrintCommandError(cmdArg->argString, NULL, errno);
        return FALSE;
    }
    return TRUE;
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
