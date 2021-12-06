#include "menu.h"
#include "shell.h"
#include "config.h"
#include "chainloader.h"
#include "cmds/cat.h"
#include "logger.h"

#define CHAR_INT 48 // to convert from unicode to regular numbers
#define SHELL_CHAR 'c'

/*char - char_t, int8_t
  int - int32_t
  short - int16_t
*/

void MainMenu()
{
    ST->ConOut->ClearScreen(ST->ConOut);
    boot_entry_s* headConfig = ParseConfig();
     
    if(headConfig == NULL)
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

    do{
        key = GetInputKey();
    }while(key.UnicodeChar != SHELL_CHAR && ((key.UnicodeChar > i + CHAR_INT) || (key.UnicodeChar < '1')));
    

    curr = GetCurrOS(key.UnicodeChar - CHAR_INT, head);

    switch(key.UnicodeChar)
    {
        case BT_CHAINLOAD:
            ChainloadImage(curr->mainPath);
            break;
        case BT_LINUX:
            printf("not avilable at the moment\n");// will append this func soon  
            break;
        case SHELL_CHAR:
            StartShell();
            break;    

    } 

          
    
}

void FailMenu()
{
    // void* funcArr[3] = {RT->ResetSystem};
    /*2 options 
    text editor
    restart or shut down
    */
    
    
    ST->ConOut->ClearScreen(ST->ConOut);
    printf("configure file is empty or incorrect!!!\n\n");
    printf("n1) open shell    (fix/change configure file)\n");
    printf("2) logger\n");
    printf("3) shut down\n");
    printf("4) restart\n");

    
    //clear buffer and read key stroke
    ST->ConIn->Reset(ST->ConIn, 0);    
    efi_input_key_t key;

    do{
        key = GetInputKey();
    }while((key.UnicodeChar < '1') || (key.UnicodeChar > '4'));
    //check if key is valid af
    
    switch(key.UnicodeChar)
    {
        case '1':
            StartShell();
            break;
        case '2':
            Logger();
            break;
        case '3':
            ShutDown();
            break;
        case '4':
            ResetComputer();
            break;
    }
    
}

void Logger()
{
 //the looger will be clear for a while
    uint8_t res = PrintFileContent(LOG_PATH);
    if (res != CMD_SUCCESS)
    {
        printf("Failed to open log file!\n");
    }

}

void ShutDown()
{
    ST->ConOut->ClearScreen(ST->ConOut);
    printf("shutdown");
    RT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
}

void ResetComputer()
{
    ST->ConOut->ClearScreen(ST->ConOut);
    printf("restarting...\n");
    BS->Stall(500000);
    RT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
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
