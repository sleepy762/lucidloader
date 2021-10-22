#include "menu.h"
#include "shell.h"
#include "config.h"
#include "config.h"

void mainMenu()
{
    //figure out what is in the config file and print it or an error
    boot_entry_s* headConfig = ParseConfig();

    if(!headConfig)
    {
        failMenu();
    }

    
}

void failMenu()
{
    void* funcArr[3] = {RT->ResetSystem};
    /*2 options 
    text editor
    restart or shut down
    */

    ST->ConOut->ClearScreen(ST->ConOut);
    printf("configure file is empty or incorrect\n");
    printf("\n\n1) text editor    (fix/change configure file)\n\n");
    printf("2) shut down\n3) restart\n\n");


    //clear buffer and read key stroke
    ST->ConIn->Reset(ST->ConIn, 0);
    efi_input_key_t key;
    //check if key is valid af
    while((ST->ConIn->ReadKeyStroke(ST->ConIn, &key) == EFI_NOT_READY) && &key <= 3 && &key >=1);




}

/*meun 
1. boot menu - shows all os/efi  and start booting proc
2. settings 
3. start shell
4. reset */