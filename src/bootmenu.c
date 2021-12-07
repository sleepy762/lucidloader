#include "bootmenu.h"

/*char - char_t, int8_t
  int - int32_t
  short - int16_t
*/

void MainMenu(void)
{
    ST->ConOut->ClearScreen(ST->ConOut);
    boot_entry_s* headConfig = ParseConfig();
     
    if (headConfig == NULL)
    {
        FailMenu();
    }
    else
    {
        SuccessMenu(headConfig);
    }
}

void SuccessMenu(boot_entry_s* head)
{
    while (TRUE)
    {
        uint8_t i = 0;
        boot_entry_s* curr = head;

        ST->ConOut->ClearScreen(ST->ConOut);
        
        while (curr != NULL)
        {  
            printf("%d. %s, %d , %s\n",++i, curr->name, curr->type, curr->mainPath);
            curr = curr->next;
        } 
        
        printf("\nfor shell press 'c'\n\n");

        //clear buffer and read key stroke
        ST->ConIn->Reset(ST->ConIn, 0);    
        efi_input_key_t key;

        do
        {
            key = GetInputKey();
        } while (key.UnicodeChar != SHELL_CHAR && 
                ((key.UnicodeChar > i + CHAR_INT) || (key.UnicodeChar < '1')));
        
        if(key.UnicodeChar == SHELL_CHAR)
        {
            StartShell();
        }
        
        curr = GetCurrOS(key.UnicodeChar - CHAR_INT, head);

        switch(curr->type)
        {
            case BT_CHAINLOAD:
                ChainloadImage(curr->mainPath);
                break;
            case BT_LINUX:
                printf("not avilable at the moment\n");// will append this func soon  
                break; 
        }
    }
}

void FailMenu(void)
{
    // void* funcArr[3] = {RT->ResetSystem};
    /*2 options 
    text editor
    restart or shut down
    */
    while (TRUE)
    {
        ST->ConOut->ClearScreen(ST->ConOut);
        printf("configure file is empty or incorrect!!!\n\n");
        printf("1) open shell    (fix/change configure file)\n");
        printf("2) logger\n");
        printf("3) shut down\n");
        printf("4) restart\n");
        
        //clear buffer and read key stroke
        ST->ConIn->Reset(ST->ConIn, 0);    
        efi_input_key_t key;

        do
        {
            key = GetInputKey();
        } while ((key.UnicodeChar < '1') || (key.UnicodeChar > '4'));
        //check if key is valid af
        
        switch(key.UnicodeChar)
        {
            case '1':
                StartShell();
                break;
            case '2':
                ShowLogFile();
                break;
            case '3':
                ShutdownDevice();
                break;
            case '4':
                RebootDevice(FALSE);
                break;
            default:
                // nothing
                break;
        }
    }
}

void ShowLogFile(void)
{
    ST->ConOut->ClearScreen(ST->ConOut);

    uint8_t res = PrintFileContent(LOG_PATH);
    if (res != CMD_SUCCESS)
    {
        printf("Failed to open log file!\n");
    }

    printf("\nPress any key to continue...");
    GetInputKey();
}

/*meun 
1. boot menu - shows all os/efi  and start booting proc
2. settings 
3. start shell
4. reset */

boot_entry_s * GetCurrOS(uint8_t numOfPartition, boot_entry_s * head)
{
    uint8_t  i = 0;
    boot_entry_s * curr = head;

    for(i = 1; i < numOfPartition; i++)
    {
        curr = curr->next;
    }
    
    return curr;
}
