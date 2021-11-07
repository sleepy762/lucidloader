#include "menu.h"
#include "shell.h"
#include "config.h"
#include "config.h"

#define CHAR_INT 48 // to convert from unicode to regular numbers

/*char - char_t, int8_t
  int - int32_t
  short - int16_t
*/

void MainMenu()
{
    ST->ConOut->ClearScreen(ST->ConOut);
    
    //figure out what is in the config file and print it or an error
    //boot_entry_s* headConfig = ParseConfig();
    
    if(1) FailMenu();

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
    if(key.UnicodeChar == '2') Logger();//need to add a looger
    if(key.UnicodeChar == '3') ShutDown();
    if(key.UnicodeChar == '4') ResetComputer();
    
}

void Logger()
{

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