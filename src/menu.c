#include "menu.h"
#include "shell.h"
#include "config.h"
#include "chainloader.h"
#include "cmds/cat.h"
#include "logger.h"

#define CHAR_INT 48 // to convert from unicode to regular numbers

/*char - char_t, int8_t
  int - int32_t
  short - int16_t
*/

void MainMenu()
{
    ST->ConOut->ClearScreen(ST->ConOut);
    boot_entry_s* headConfig = NULL;
    //figure out what is in the config file and print it or an error    
    headConfig = ParseConfig();
     
    if(!headConfig){
         FailMenu();
    }
    else{
        SuccssesMenu(headConfig);
    }

}

void SuccssesMenu(boot_entry_s* head)
{
    uint8_t i = 0;
    boot_entry_s* curr = head;

    ST->ConOut->ClearScreen(ST->ConOut);
    
    while (curr)
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
    }while(!((key.UnicodeChar == 'c') || (key.UnicodeChar <= i + CHAR_INT) && (key.UnicodeChar >= '1')));
    

    curr = GetCurrOS(key.UnicodeChar - CHAR_INT, head);
    

    if(curr->type == 1) ChainloadImage(curr->mainPath);
    if(curr->type == 2) printf("not avilable at the moment\n");// will append this func soon     
    if(key.UnicodeChar == 'c') StartShell();
}

void FailMenu()
{
    // void* funcArr[3] = {RT->ResetSystem};
    /*2 options 
    text editor
    restart or shut down
    */
    
    
    ST->ConOut->ClearScreen(ST->ConOut);
    printf("configure file is empty or incorrect!!!");
    printf("\n\n1) open shell    (fix/change configure file)\n");
    printf("2) logger\n3) shut down\n4) restart\n");

    
    //clear buffer and read key stroke
    ST->ConIn->Reset(ST->ConIn, 0);    
    efi_input_key_t key;

    do{
    while ((ST->ConIn->ReadKeyStroke(ST->ConIn, &key)) == EFI_NOT_READY);     
    }while((key.UnicodeChar - CHAR_INT >= 1) && ((key.UnicodeChar - CHAR_INT) > 4));
    //check if key is valid af
    
    if(key.UnicodeChar == '1') StartShell();
    if(key.UnicodeChar == '2') Logger();
    if(key.UnicodeChar == '3') ShutDown();
    if(key.UnicodeChar == '4') ResetComputer();
    
}

void Logger()
{
 //the looger will be clear for a while
    ST->ConOut->ClearScreen(ST->ConOut);

    char_t* envPath = "\\EFI\\apps\\";

    cmd_args_s catVar;
    catVar.argString = "ezboot-log.txt";
    catVar.next = NULL;

    CatCmd(&catVar, &envPath); // using the cat cmd to open the file and print it
   
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
    ST->BootServices->Stall(500000);
    ST->RuntimeServices->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
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
