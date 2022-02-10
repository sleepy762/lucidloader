#include "cmds/passwd.h"

uint8_t PasswdCmd(cmd_args_s** args, char_t** currPathPtr)
{
    FILE* passFile = fopen(PASS_FILE_PATH, "w");
    if (passFile == NULL)
    {
        PrintCommandError("passwd", NULL, CMD_GENERAL_FILE_OPENING_ERROR);
        return CMD_SUCCESS;
    }

    //char_t* buffer[] = {0};
    //GetInputString(buffer, MAX_PASS_LEN, TRUE);

    //Enc((uint8_t*)buffer, (uint8_t*)buffer);

    fclose(passFile);
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
