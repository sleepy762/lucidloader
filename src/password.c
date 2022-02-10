#include "password.h"

boolean_t CheckPassword(char_t password[])
{
    char_t* buffer = GetFileContent(PASS_FILE_PATH);

    //enc conventions we set(just use the msg as key)
    enc(password,password);
    
    if(strcmp(password, buffer) == 0)
    {
        return TRUE;
    }

    return FALSE;
}
