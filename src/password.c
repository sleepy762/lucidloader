#include "password.h"
#include "bootutils.h"
#include "shellutils.h"
#include "encryption.h"
#include "bootmenu.h"
#include "logger.h"

#define MAX_PASS_LEN (16)
#define SLEEP_LENGTH_FOR_BAD_PASS (2)


// Returns TRUE if the login succeeded, FALSE otherwise
boolean_t ShellLoginWithPassword()
{
    char_t* encPassword = GetFileContent(PASS_FILE_PATH, NULL);
    // Check if password doesn't exist
    if((encPassword == NULL && errno == ENOENT) || strlen(encPassword) == 0)
    {
        return TRUE;
    }
    else if (encPassword == NULL)
    {
        Log(LL_ERROR, 0, "Failed to open password file.");
        return FALSE;
    }

    ST->ConOut->ClearScreen(ST->ConOut);

    PrintBootloaderVersion();
    printf("The shell is protected with a password.\n"
           "Enter the shell password: ");

    char_t password[MAX_PASS_LEN + 1] = {0};
    GetInputString(password, MAX_PASS_LEN, TRUE);

    HashString(password);

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

    // Don't write empty password, but consider it successful
    if (strlen(buffer) == 0)
    {
        fclose(out);
        return TRUE;
    }
	
    HashString(buffer);
    
    // Store the encrypted password in a file
    size_t ret = fwrite(buffer, 1, MAX_PASS_LEN, out);
    if (ret == 0)
    {
        fclose(out);
        return FALSE;
    }
    fclose(out);
    
    Log(LL_INFO, 0, "New shell password has been set.");
    return TRUE;
}
