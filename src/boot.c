#include <uefi.h>
#include "logger.h"
#include "chainloader.h"
#include "config.h"
#include "shell.h"
#include "menu.h"

int main(int argc, char** argv)
{
    // Global status of the bootloader
    
    if(!InitLogger())
    {
        printf("Failed to initialize logger. Logging disabled.\n");
    }

    MainMenu();

    // This should never be reached
    return 0;
}
