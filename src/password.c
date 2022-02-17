#include "password.h"

boolean_t CheckPassword(char_t password[])
{
    char_t* buffer = GetFileContent(PASS_FILE_PATH);

    //enc conventions we set(just use the msg as key)
    Enc((uint8_t*)password, (uint8_t*)password);
    
    if (strcmp(password, buffer) == 0)
    {
        return TRUE;
    }

    return FALSE;
}

boolean_t EnterPassword()
{
    boolean_t ans;
    char_t buffer[MAX_PASS_LEN] = {0};
    GetInputString(buffer, MAX_PASS_LEN, 1);

    FILE *out = fopen(PASS_FILE_PATH,"w");
	
    Enc(buffer,buffer);
    
    ans = fwrite(buffer, 1, MAX_PASS_LEN,out);
	
    fclose(out);

    return ans;
}
