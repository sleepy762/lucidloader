#include "password.h"

// Returns TRUE if the login succeeded, FALSE otherwise
boolean_t ShellLoginWithPassword()
{
    char_t* encPassword = GetFileContent(PASS_FILE_PATH);
    if(encPassword == NULL && errno == ENOENT) // Password doesn't exist
    {
        return TRUE;
    }

    ST->ConOut->ClearScreen(ST->ConOut);
    Log(LL_INFO, 0, "Asking for shell password...");

    PrintBootloaderVersion();
    printf("The shell is protected with a password.\n");
    printf("Enter the shell password: ");

    char_t password[MAX_PASS_LEN]= {0};
    GetInputString(password, MAX_PASS_LEN, TRUE);

    //enc conventions we set(just use the msg as key)
    Enc((uint8_t*)password, (uint8_t*)password);

    if (strcmp(password, encPassword) == 0)
    {
        // The correct password was entered
        Log(LL_INFO, 0, "Shell login succeeded.");
        return TRUE;
    }

    Log(LL_INFO, 0, "Shell login failed with an incorrect password.");
    printf("\nIncorrect password.\n");
    sleep(SLEEP_LENGTH_FOR_BAD_PASS);

    return FALSE;
}

// Returns TRUE if succeeded, FALSE otherwise
boolean_t CreateShellPassword()
{
    // Open the password file
    FILE *out = fopen(PASS_FILE_PATH,"w");
    if (out == NULL)
    {
        return FALSE;
    }

    char_t buffer[MAX_PASS_LEN] = {0};

    // Get and encrypt the password
    printf("Enter a new password: ");
    GetInputString(buffer, MAX_PASS_LEN, TRUE);
	
    Enc((uint8_t*)buffer, (uint8_t*)buffer);
    
    // Store the encrypted password in a file
    size_t ret = fwrite(buffer, 1, MAX_PASS_LEN, out);
    if (ret == 0)
    {
        return FALSE;
    }
    fclose(out);

    return TRUE;
}
